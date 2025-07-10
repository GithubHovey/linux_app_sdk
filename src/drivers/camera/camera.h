#ifndef CAMERA_H
#define CAMERA_H

#include "utils.h"
class Camera {
public:
    struct Frame {
        void* data;
        size_t size;
        uint8_t index;
        std::atomic<int> ref_count;
        struct timeval timestamp;

        Frame() = default;
        Frame(const Frame&) = delete;            // 禁用拷贝构造
        Frame& operator=(const Frame&) = delete; // 禁用拷贝赋值
        
        // 自定义移动构造函数
        Frame(Frame&& other) noexcept 
            : data(other.data),
              size(other.size),
              index(other.index),
              ref_count(other.ref_count.load()), // 显式加载原子值，不这么做的话没法push pop std::queue
              timestamp(other.timestamp) {
            other.data = nullptr;
            other.size = 0;
            other.index = 0;
        }
        
        Frame& operator=(Frame&&) = default;     // 允许移动赋值
        };

    explicit Camera(const std::string& device, std::string logfile, uint32_t width, uint32_t height, uint32_t fps, uint8_t buffer_numb = 10);
    ~Camera();

    int init();
        // Frame capture
    bool getFrame(Frame& frame, uint32_t timeout_ms = 5000);
    bool returnFrame(const Frame& frame);
    void CaptureThread();
    Frame& GetLatestFrame();
private:
    // Camera controls
    bool setBrightness(int32_t value);
    bool getBrightness(int32_t& value) const;
    bool setContrast(int32_t value);
    bool getContrast(int32_t& value) const;
    // Add more controls as needed...


    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif // CAMERA_H