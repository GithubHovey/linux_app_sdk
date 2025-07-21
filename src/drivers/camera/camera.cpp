#include "camera.h"
#include "log/logmanager.h"
#ifdef USE_RKMEDIA
#include "rkcapture.h"
#else
#include "v4l2_capture.h"
#endif

#include <sys/time.h>
#define BUFFER_NUMB 10
#define QUEUE_POP_THRESHOLD 2
class Camera::Impl {
public:
    explicit Impl(const YAML::Node& config)
    {
        capture_list.clear();
        for (const auto& channel_entry : config["channel_list"]) {
            const auto& capture_config = channel_entry.second;
            
            if (capture_config["enable"].as<bool>()) {
                auto channel = std::make_unique<CaptureChannel>();
                channel->name = capture_config["name"].as<std::string>();
                channel->width = capture_config["width"].as<uint16_t>();
                channel->height = capture_config["height"].as<uint16_t>();
                channel->fps = capture_config["fps"].as<uint16_t>();
                channel->rotation = capture_config["rotation"].as<uint16_t>();
                channel->fmt = capture_config["fmt"].as<std::string>();
                if (capture_config["fix_width"] && capture_config["fix_height"]) {
                    channel->fix_width = capture_config["fix_width"].as<uint16_t>();
                    channel->fix_height = capture_config["fix_height"].as<uint16_t>();
                } else {
                    channel->fix_width = channel->width;
                    channel->fix_height = channel->height;
                }
                channel->capture = std::make_unique<RKCapture>(channel->name,capture_config["buf_cnt"].as<uint8_t>());       
                capture_list.push_back(std::move(channel));

            } else {
                capture_list.push_back(nullptr);
            }
        }
        logger = LogManager::GetLogger(config["name"].as<std::string>());
        logger->info("---------------");
        logger->info("camera driver init...");
    }
    struct CaptureChannel
    {
        std::string name;
        uint16_t width;
        uint16_t height;
        uint16_t fps;  // Default frame rate
        uint16_t rotation;
        std::string fmt;
        uint16_t fix_width;
        uint16_t fix_height;
        std::queue<Frame> frame_queue;
        std::queue<Frame> frame_return_queue;
        std::mutex queue_mutex;
        std::unique_ptr<RKCapture> capture;
    };
    std::vector<std::unique_ptr<CaptureChannel>> capture_list;
    std::shared_ptr<spdlog::logger> logger;
    struct {
        uint32_t current_fps;                  // 目标帧率
        std::chrono::steady_clock::time_point start_time;  // 帧开始时间
        uint32_t counter = 0;           // 帧计数器
    } fps_manager;
};

// Camera::Camera(const std::string& device,std::string logfile,  uint32_t width, uint32_t height, uint32_t fps,uint8_t buffer_numb) : impl_(std::make_unique<Impl>(device,logfile, width,height,fps)) {}
Camera::Camera(YAML::Node& config):
    impl_(std::make_unique<Impl>(config))
    {
        
    }
Camera::~Camera() = default;

int Camera::init() 
{
#ifdef USE_RKMEDIA
    return 0;
#else
    bool ret = impl_->capture.Open(); 
    if(!ret) return -1;
    ret = impl_->capture.CheckCap(); //确认设备支持视频采集
    if(!ret) return -2;
    ret = impl_->capture.CheckSupportFormat(); //查看v4l2设备支持的格式
    if(!ret) return -3;
    ret = impl_->capture.SetFormat(impl_->width, impl_->height, impl_->pixel_format);
    if(!ret) return -4;
    ret = impl_->capture.SetFrameRate(impl_->fps);
    if(!ret) return -5;
    ret = impl_->capture.InitBuffers(impl_->buffer_numb);
    if(!ret) return -6;
    return 0;
#endif
    
}

void Camera::CaptureThread()
{
    bool ret = impl_->capture.StartStream();
    if(!ret) return ;
#ifdef USE_RKMEDIA

#else
    V4L2Capture::Buffer buffer;
#endif

    struct timeval timestamp;
    auto last_fps_update = std::chrono::steady_clock::now();
    while(true)
    {
        Frame frame;
#ifdef USE_RKMEDIA
        if(impl_->capture.captureFrame(frame.data, frame.size, frame.index, frame.timestamp)) 
#else
        if(impl_->capture.captureFrame(buffer,timestamp)) 
#endif
        { //阻塞等待新帧
 #ifdef USE_RKMEDIA
            
 #else

            frame.data = buffer.start;
            frame.size = buffer.length;
            frame.index = buffer.index;
            frame.timestamp = timestamp;
#endif
            frame.ref_count = 0;
            {
                std::lock_guard<std::mutex> lock(impl_->queue_mutex);
                impl_->frame_queue.push(std::move(frame));
                while (impl_->frame_queue.size() > QUEUE_POP_THRESHOLD) {//旧帧（队序列>5）全部塞回内核
                    //注意：这里没有给return队列上锁，不要在其他地方访问return队列
                    if(impl_->frame_queue.front().ref_count.load() == 0)
                    {
                        impl_->frame_return_queue.push(std::move(impl_->frame_queue.front()));
                        impl_->frame_queue.pop();
                    }
                }
            }
            while (!impl_->frame_return_queue.empty()) {
                if(!impl_->capture.returnFrame(impl_->frame_return_queue.front().index))
                {
                    
                }
                impl_->frame_return_queue.pop();
            }
            impl_->fps_manager.counter++;
        }
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update).count();

        if(elapsed >= 1000) {
            double actual_fps = impl_->fps_manager.counter / (elapsed / 1000.0);
            // LOG(INFO) << "FPS: " << actual_fps;
            impl_->logger->info("FPS = {}",actual_fps);
            last_fps_update = now;
            impl_->fps_manager.counter = 0;
        }
    }

    impl_->capture.StopStream();
    impl_->capture.close();
}

Camera::Frame&  Camera::GetLatestFrame()
{
    std::lock_guard<std::mutex> lock(impl_->queue_mutex);
    return impl_->frame_queue.back();
}