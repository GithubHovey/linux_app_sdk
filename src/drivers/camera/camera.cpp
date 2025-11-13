#include "camera.h"
#include "log/logmanager.h"
#ifdef USE_RKMEDIA
#include "rkcapture.h"
#else
#include "v4l2_capture.h"
#endif
#include <sys/time.h>
#define BUFFER_NUMB 10
#define QUEUE_POP_THRESHOLD 5
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
#ifdef   USE_RKMEDIA             
                channel->capture = std::make_unique<RKCapture>(channel->name,capture_config["bufcnt"].as<uint16_t>());   
#else
                channel->capture = std::make_unique<V4L2Capture>(channel->name,capture_config["bufcnt"].as<uint16_t>());
#endif
                channel->logger = LogManager::GetLogger(channel->name);
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
#ifdef USE_RKMEDIA
        std::unique_ptr<RKCapture> capture;
#else
        std::unique_ptr<V4L2Capture> capture;
#endif
        struct {
            uint32_t current_fps;                  // 目标帧率
            std::chrono::steady_clock::time_point start_time;  // 帧开始时间
            uint32_t counter = 0;           // 帧计数器
        } fps_manager;
        std::shared_ptr<spdlog::logger> logger;
    };
    std::vector<std::shared_ptr<CaptureChannel>> capture_list;
    std::shared_ptr<spdlog::logger> logger;

};

// Camera::Camera(const std::string& device,std::string logfile,  uint32_t width, uint32_t height, uint32_t fps,uint8_t buffer_numb) : impl_(std::make_unique<Impl>(device,logfile, width,height,fps)) {}
Camera::Camera(const YAML::Node& config):
    impl_(std::make_unique<Impl>(config))
    {
        
    }
Camera::~Camera() = default;

int Camera::init() 
{
    for(auto& capture_channel : impl_->capture_list)
    {
        if(!capture_channel)
        {
            continue;
        }
        std::thread t([this, channel = capture_channel.get()] {
            bool ret = channel->capture->init(
                (uint32_t)channel->width,
                (uint32_t)channel->height,
                (uint32_t)channel->fps, // Default frame rate
                channel->fmt,
                (uint32_t)channel->rotation,
                (uint32_t)channel->fix_width,
                (uint32_t)channel->fix_height
                );
            if(!ret) 
            {
                channel->logger->error("capture init failed,ret = {}",ret);
                return;
            }
            ret = channel->capture->StartStream();
            if(!ret) 
            {
                channel->logger->error("capture start stream failed,ret = {}",ret);
                return;
            }
            channel->logger->info("capture init success");

            struct timeval timestamp;
            auto last_fps_update = std::chrono::steady_clock::now();
            while(true) {
                Frame frame;
                channel->logger->debug("Capturing frame...");
                //从内核队列中获得帧
                if(channel->capture->captureFrame(frame.data, frame.size, frame.index, frame.timestamp)) {
                    frame.ref_count = 0;// 新捕获的帧引用计数初始化为0
                    {
                        std::lock_guard<std::mutex> lock(channel->queue_mutex);
                        channel->frame_queue.push(std::move(frame));// 将帧移动到队列中
// 检查队列是否超过阈值，若超过则将帧移动到返回队列,注意要判断引用计数是否为0，如果不为0说明这一帧正在被使用，不能移动到返回队列
                        while (channel->frame_queue.size() > QUEUE_POP_THRESHOLD) {
                            if(channel->frame_queue.front().ref_count.load() == 0) {
                                // 移动到返回队列
                                channel->frame_return_queue.push(std::move(channel->frame_queue.front()));
                                channel->frame_queue.pop();
                            }
                        }
                    }
                    // 如果返回队列有帧，则将帧的所有权交回给内核
                    while (!channel->frame_return_queue.empty()) {
                        channel->logger->debug("Returning frame index {} to kernel...", channel->frame_return_queue.front().index);
                        if(!channel->capture->returnFrame(channel->frame_return_queue.front().index)) {
                            // Handle error if needed
                        }
                        channel->frame_return_queue.pop();
                    }
                    channel->fps_manager.counter++;
                }
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_fps_update).count();

                if(elapsed >= 1000) {
                    double actual_fps = channel->fps_manager.counter / (elapsed / 1000.0);
                    channel->logger->info("FPS = {}",actual_fps);
                    last_fps_update = now;
                    channel->fps_manager.counter = 0;
                }
            }
            channel->capture->StopStream();
            channel->capture->close();
        });
        t.detach(); // or store thread
    }
    return 0;

    
}
Camera::Frame&  Camera::GetLatestFrame(uint8_t port)
{
    std::lock_guard<std::mutex> lock(impl_->capture_list[port]->queue_mutex);
    Frame& frame = impl_->capture_list[port]->frame_queue.back();
    ++frame.ref_count;
    return frame;
}
int Camera::RleaseFrame(uint8_t port, Frame& frame)
{
    std::lock_guard<std::mutex> lock(impl_->capture_list[port]->queue_mutex);
    --frame.ref_count;
    return 0;
}