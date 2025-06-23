#include "v4l2_capture.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <cstring>
#include <stdexcept>
#include <sstream>
#include <iostream>

V4L2Capture::V4L2Capture(const std::string& device)
    : device_path_(device), fd_(-1), is_streaming_(false),
      width_(0), height_(0), pixel_format_(0) {}

V4L2Capture::~V4L2Capture() {
    if (is_streaming_) stopStream();
    if (fd_ != -1) close();
}

bool V4L2Capture::open() {
    if (isOpened()) return true;

    fd_ = ::open(device_path_.c_str(), O_RDWR | O_NONBLOCK);
    if (fd_ < 0) {
        return false;
    }

    // 查询设备能力
    v4l2_capability cap;
    if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) == -1) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        ::close(fd_);
        fd_ = -1;
        return false;
    }

    return true;
}

void V4L2Capture::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
    is_streaming_ = false;
}

bool V4L2Capture::isOpened() const {
    return fd_ != -1;
}

V4L2Capture::DeviceInfo V4L2Capture::queryDeviceInfo() const {
    DeviceInfo info;
    if (!isOpened()) return info;

    v4l2_capability cap;
    if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) == 0) {
        info.driver = reinterpret_cast<char*>(cap.driver);
        info.card = reinterpret_cast<char*>(cap.card);
        info.bus_info = reinterpret_cast<char*>(cap.bus_info);
        
        std::ostringstream oss;
        oss << ((cap.version >> 16) & 0xFF) << "."
            << ((cap.version >> 8) & 0xFF) << "."
            << (cap.version & 0xFF);
        info.version = oss.str();
        
        info.capabilities = cap.capabilities;
    }

    return info;
}

std::vector<V4L2Capture::PixelFormat> V4L2Capture::enumFormats() const {
    std::vector<PixelFormat> formats;
    if (!isOpened()) return formats;

    v4l2_fmtdesc fmt_desc = {};
    fmt_desc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd_, VIDIOC_ENUM_FMT, &fmt_desc) == 0) {
        PixelFormat pf;
        pf.fourcc = fmt_desc.pixelformat;
        pf.description = reinterpret_cast<char*>(fmt_desc.description);
        formats.push_back(pf);
        fmt_desc.index++;
    }

    return formats;
}

bool V4L2Capture::setFormat(uint32_t width, uint32_t height, uint32_t pixfmt) {
    if (!isOpened()) return false;

    v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = pixfmt;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) == -1) {
        return false;
    }

    // 检查实际设置的格式
    if (fmt.fmt.pix.pixelformat != pixfmt) {
        return false;
    }

    width_ = fmt.fmt.pix.width;
    height_ = fmt.fmt.pix.height;
    pixel_format_ = fmt.fmt.pix.pixelformat;

    return true;
}

bool V4L2Capture::getFormat(uint32_t& width, uint32_t& height, uint32_t& pixfmt) const {
    if (!isOpened()) return false;

    v4l2_format fmt = {};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (ioctl(fd_, VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    width = fmt.fmt.pix.width;
    height = fmt.fmt.pix.height;
    pixfmt = fmt.fmt.pix.pixelformat;

    return true;
}

bool V4L2Capture::startStream(uint32_t buffer_count) {
    if (!isOpened() || is_streaming_) return false;

    if (!initBuffers(buffer_count)) {
        return false;
    }

    // 将所有缓冲区加入队列
    for (uint32_t i = 0; i < buffers_.size(); ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
            cleanupBuffers();
            return false;
        }
    }

    // 开始流
    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) == -1) {
        cleanupBuffers();
        return false;
    }

    is_streaming_ = true;
    return true;
}

bool V4L2Capture::stopStream() {
    if (!is_streaming_) return true;

    v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(fd_, VIDIOC_STREAMOFF, &type) == -1) {
        return false;
    }

    cleanupBuffers();
    is_streaming_ = false;
    return true;
}

bool V4L2Capture::isStreaming() const {
    return is_streaming_;
}

bool V4L2Capture::captureFrame(Frame& frame, uint32_t timeout_ms) {
    if (!is_streaming_) return false;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);

    timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    int r = select(fd_ + 1, &fds, nullptr, nullptr, &tv);
    if (r == 0) return false; // 超时
    if (r == -1) return false; // 错误

    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
        return false;
    }

    if (buf.index >= buffers_.size()) {
        // 异常情况，重新加入队列
        ioctl(fd_, VIDIOC_QBUF, &buf);
        return false;
    }

    frame.data = buffers_[buf.index].start;
    frame.size = buf.bytesused;
    frame.index = buf.index;
    frame.timestamp = buf.timestamp;

    return true;
}

bool V4L2Capture::returnFrame(const Frame& frame) {
    if (!is_streaming_) return false;

    v4l2_buffer buf = {};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = frame.index;

    if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
        return false;
    }

    return true;
}

bool V4L2Capture::setControl(uint32_t ctrl_id, int32_t value) {
    if (!isOpened()) return false;

    v4l2_control ctrl = {};
    ctrl.id = ctrl_id;
    ctrl.value = value;

    if (ioctl(fd_, VIDIOC_S_CTRL, &ctrl) == -1) {
        return false;
    }

    return true;
}

bool V4L2Capture::getControl(uint32_t ctrl_id, int32_t& value) const {
    if (!isOpened()) return false;

    v4l2_control ctrl = {};
    ctrl.id = ctrl_id;

    if (ioctl(fd_, VIDIOC_G_CTRL, &ctrl) == -1) {
        return false;
    }

    value = ctrl.value;
    return true;
}

bool V4L2Capture::initBuffers(uint32_t buffer_count) {
    if (buffer_count < 2) return false;

    // 请求缓冲区
    v4l2_requestbuffers req = {};
    req.count = buffer_count;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_REQBUFS, &req) == -1) {
        return false;
    }

    if (req.count < 2) {
        return false;
    }

    buffers_.resize(req.count);

    // 映射缓冲区
    for (uint32_t i = 0; i < req.count; ++i) {
        v4l2_buffer buf = {};
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = i;

        if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
            cleanupBuffers();
            return false;
        }

        buffers_[i].start = mmap(nullptr, buf.length,
                                PROT_READ | PROT_WRITE,
                                MAP_SHARED,
                                fd_, buf.m.offset);
        buffers_[i].length = buf.length;
        buffers_[i].index = i;

        if (buffers_[i].start == MAP_FAILED) {
            cleanupBuffers();
            return false;
        }
    }

    return true;
}

void V4L2Capture::cleanupBuffers() {
    for (auto& buffer : buffers_) {
        if (buffer.start != nullptr && buffer.start != MAP_FAILED) {
            munmap(buffer.start, buffer.length);
        }
    }
    buffers_.clear();
}