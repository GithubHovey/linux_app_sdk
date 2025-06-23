#ifndef V4L2_CAPTURE_H
#define V4L2_CAPTURE_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <linux/videodev2.h>

class V4L2Capture {
public:
    // 帧数据结构
    struct Frame {
        void* data;
        size_t size;
        uint32_t index;
        struct timeval timestamp;
    };

    // 设备信息
    struct DeviceInfo {
        std::string driver;
        std::string card;
        std::string bus_info;
        std::string version;
        uint32_t capabilities;
    };

    // 像素格式
    struct PixelFormat {
        uint32_t fourcc;
        std::string description;
    };

    explicit V4L2Capture(const std::string& device = "/dev/video0");
    ~V4L2Capture();

    // 禁止拷贝
    V4L2Capture(const V4L2Capture&) = delete;
    V4L2Capture& operator=(const V4L2Capture&) = delete;

    // 设备操作
    bool open();
    void close();
    bool isOpened() const;

    // 设备信息
    DeviceInfo queryDeviceInfo() const;
    std::vector<PixelFormat> enumFormats() const;

    // 格式设置
    bool setFormat(uint32_t width, uint32_t height, uint32_t pixfmt);
    bool getFormat(uint32_t& width, uint32_t& height, uint32_t& pixfmt) const;

    // 流控制
    bool startStream(uint32_t buffer_count = 4);
    bool stopStream();
    bool isStreaming() const;

    // 帧捕获
    bool captureFrame(Frame& frame, uint32_t timeout_ms = 5000);
    bool returnFrame(const Frame& frame);

    // 控制接口
    bool setControl(uint32_t ctrl_id, int32_t value);
    bool getControl(uint32_t ctrl_id, int32_t& value) const;

private:
    struct Buffer {
        void* start;
        size_t length;
        uint32_t index;
    };

    bool initBuffers(uint32_t buffer_count);
    void cleanupBuffers();

    std::string device_path_;
    int fd_;
    bool is_streaming_;
    std::vector<Buffer> buffers_;
    uint32_t width_;
    uint32_t height_;
    uint32_t pixel_format_;
};

#endif // V4L2_CAPTURE_H

/*how to use*/
/*
V4L2Capture capture("/dev/video0");
capture.open();
capture.setFormat(1280, 720, V4L2_PIX_FMT_MJPEG);
capture.startStream();


V4L2Capture::Frame frame;
while (capture.captureFrame(frame)) {
    // 处理帧数据
    processFrame(frame.data, frame.size);
    capture.returnFrame(frame);
}

capture.stopStream();
capture.close();
*/