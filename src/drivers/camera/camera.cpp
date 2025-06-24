#include "camera.h"
#include "v4l2_capture.h"
#include <memory>
#include <sys/time.h>

class Camera::Impl {
public:
    explicit Impl(const std::string& device, uint32_t width, uint32_t height, uint32_t fps, uint8_t buffer_numb = 10, uint32_t pixel_format = V4L2_PIX_FMT_RGB24)
        : capture(device), width(width), height(height), fps(fps), buffer_numb(buffer_numb), pixel_format(pixel_format){}

    V4L2Capture capture;
    uint32_t width;
    uint32_t height;
    uint32_t fps;  // Default frame rate
    uint32_t pixel_format;
    uint8_t buffer_numb;
};

Camera::Camera(const std::string& device, uint32_t width, uint32_t height, uint32_t fps,uint8_t buffer_numb) : impl_(std::make_unique<Impl>(device,width,height,fps)) {}
Camera::~Camera() = default;

int Camera::init() 
{
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
    ret = impl_->capture.StartStream();
    if(!ret) return -7;
    return 0;
}


bool Camera::setBrightness(int32_t value) {
    return impl_->capture.setControl(V4L2_CID_BRIGHTNESS, value);
}

bool Camera::getBrightness(int32_t& value) const {
    return impl_->capture.getControl(V4L2_CID_BRIGHTNESS, value);
}

bool Camera::setContrast(int32_t value) {
    return impl_->capture.setControl(V4L2_CID_CONTRAST, value);
}

bool Camera::getContrast(int32_t& value) const {
    return impl_->capture.getControl(V4L2_CID_CONTRAST, value);
}