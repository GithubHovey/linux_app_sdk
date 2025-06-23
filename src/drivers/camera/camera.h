#ifndef CAMERA_H
#define CAMERA_H

#include <string>
#include <vector>
#include <cstdint>

class Camera {
public:
    struct Frame {
        void* data;
        size_t size;
        uint64_t timestamp_us; // microseconds since epoch
    };

    struct Format {
        uint32_t width;
        uint32_t height;
        uint32_t pixel_format; // V4L2_PIX_FMT_*
        std::string format_name;
    };

    explicit Camera(const std::string& device = "/dev/video0");
    ~Camera();

    // Device operations
    bool open();
    void close();
    bool isOpened() const;

    // Format operations
    std::vector<Format> getSupportedFormats() const;
    bool setFormat(uint32_t width, uint32_t height, uint32_t pixel_format);
    Format getCurrentFormat() const;

    // Stream control
    bool startStream(uint32_t buffer_count = 4);
    bool stopStream();
    bool isStreaming() const;

    // Frame capture
    bool getFrame(Frame& frame, uint32_t timeout_ms = 5000);
    bool returnFrame(const Frame& frame);

    // Camera controls
    bool setBrightness(int32_t value);
    bool getBrightness(int32_t& value) const;
    bool setContrast(int32_t value);
    bool getContrast(int32_t& value) const;
    // Add more controls as needed...

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

#endif // CAMERA_H