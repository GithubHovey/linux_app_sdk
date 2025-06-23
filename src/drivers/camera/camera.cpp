#include "camera.h"
#include "v4l2_capture.h"
#include <memory>
#include <sys/time.h>

class Camera::Impl {
public:
    explicit Impl(const std::string& device) : capture_(device) {}

    V4L2Capture capture_;
};

Camera::Camera(const std::string& device) : impl_(std::make_unique<Impl>(device)) {}
Camera::~Camera() = default;

bool Camera::open() { return impl_->capture_.open(); }
void Camera::close() { impl_->capture_.close(); }
bool Camera::isOpened() const { return impl_->capture_.isOpened(); }

std::vector<Camera::Format> Camera::getSupportedFormats() const {
    std::vector<Format> formats;
    if (!isOpened()) return formats;

    auto v4l2_formats = impl_->capture_.enumFormats();
    for (const auto& fmt : v4l2_formats) {
        Format f;
        f.pixel_format = fmt.fourcc;
        f.format_name = fmt.description;
        formats.push_back(f);
    }
    return formats;
}

bool Camera::setFormat(uint32_t width, uint32_t height, uint32_t pixel_format) {
    return impl_->capture_.setFormat(width, height, pixel_format);
}

Camera::Format Camera::getCurrentFormat() const {
    Format fmt;
    uint32_t width, height, pixfmt;
    if (impl_->capture_.getFormat(width, height, pixfmt)) {
        fmt.width = width;
        fmt.height = height;
        fmt.pixel_format = pixfmt;
        
        auto formats = impl_->capture_.enumFormats();
        for (const auto& f : formats) {
            if (f.fourcc == pixfmt) {
                fmt.format_name = f.description;
                break;
            }
        }
    }
    return fmt;
}

bool Camera::startStream(uint32_t buffer_count) {
    return impl_->capture_.startStream(buffer_count);
}

bool Camera::stopStream() { return impl_->capture_.stopStream(); }
bool Camera::isStreaming() const { return impl_->capture_.isStreaming(); }

bool Camera::getFrame(Frame& frame, uint32_t timeout_ms) {
    V4L2Capture::Frame v4l2_frame;
    if (!impl_->capture_.captureFrame(v4l2_frame, timeout_ms)) {
        return false;
    }

    frame.data = v4l2_frame.data;
    frame.size = v4l2_frame.size;
    frame.timestamp_us = v4l2_frame.timestamp.tv_sec * 1000000ULL + 
                         v4l2_frame.timestamp.tv_usec;
    return true;
}

bool Camera::returnFrame(const Frame& frame) {
    V4L2Capture::Frame v4l2_frame;
    v4l2_frame.index = frame.index; // Assuming Frame contains index
    return impl_->capture_.returnFrame(v4l2_frame);
}

bool Camera::setBrightness(int32_t value) {
    return impl_->capture_.setControl(V4L2_CID_BRIGHTNESS, value);
}

bool Camera::getBrightness(int32_t& value) const {
    return impl_->capture_.getControl(V4L2_CID_BRIGHTNESS, value);
}

bool Camera::setContrast(int32_t value) {
    return impl_->capture_.setControl(V4L2_CID_CONTRAST, value);
}

bool Camera::getContrast(int32_t& value) const {
    return impl_->capture_.getControl(V4L2_CID_CONTRAST, value);
}