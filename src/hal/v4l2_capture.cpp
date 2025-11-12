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

V4L2Capture::V4L2Capture(std::string name, uint8_t buf_cnt)
    : device_path_(name), fd_(-1), is_streaming_(false), buffer_numb(buf_cnt)
    {
        logger = LogManager::GetLogger("vision");
        logger->info("---------------");
        logger->info("v4l2 devices register...");
    }

V4L2Capture::~V4L2Capture() {
    if (is_streaming_) StopStream();
    if (fd_ != -1) close();
}
bool V4L2Capture::init(uint32_t width, uint32_t height, uint32_t fps, std::string outputFormat, uint32_t rotation, uint32_t outputWidth, uint32_t outputHeight)
{
    bool ret = Open(); 
    if(!ret) return false;
    ret = CheckCap(); //确认设备支持视频采集
    if(!ret) return false;
    ret = CheckSupportFormat(); //查看v4l2设备支持的格式
    if(!ret) return false;
    uint32_t pixel_format;
    if(outputFormat == "NV12")
        pixel_format = V4L2_PIX_FMT_NV12;
    else if(outputFormat == "RGB888")
        pixel_format = V4L2_PIX_FMT_RGB24;
    else
        return false;
    ret = SetFormat(width, height, pixel_format);
    if(!ret) return false;
    ret = SetFrameRate(fps);
    if(!ret) return false;
    ret = InitBuffers(buffer_numb);
    if(!ret) return false;
    logger->info("init v4l2 device success!");
    return true;
}
bool V4L2Capture::Open() {
    if (isOpened()) return true;

    fd_ = ::open(device_path_.c_str(), O_RDWR | O_NONBLOCK);
    if (fd_ < 0) {
        logger->error("open {} fail!",device_path_);
        return false;
    }
    logger->info("open {} success!", device_path_);
    return true;
}
    // 查询设备能力
bool V4L2Capture::CheckCap()
{
    v4l2_capability cap;
    if (ioctl(fd_, VIDIOC_QUERYCAP, &cap) == -1) {
        ::close(fd_);
        fd_ = -1;
        logger->error("VIDIOC_QUERYCAP failed for device: {}", device_path_);
        return false;
    }
    
    logger->info("Device: {}", device_path_);
    logger->info("Driver: {}", reinterpret_cast<const char*>(cap.driver));
    logger->info("Card: {}", reinterpret_cast<const char*>(cap.card));
    logger->info("Capabilities: 0x{:x}", cap.capabilities);
    
    // 检查是否支持视频捕获（单平面或多平面）
    bool supports_video_capture = (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) ||
                                  (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE);
    
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) {
        logger->info("✓ Supports VIDEO_CAPTURE (single-planar)");
        buffer_type_ = V4L2_BUF_TYPE_VIDEO_CAPTURE;  // 设置为单平面类型
    }
    
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) {
        logger->info("✓ Supports VIDEO_CAPTURE_MPLANE (multi-planar)");
        buffer_type_ = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;  // 设置为多平面类型
    }
    
    if (!supports_video_capture) {
        ::close(fd_);
        fd_ = -1;
        logger->error("Device {} does not support video capture capability (neither single-planar nor multi-planar)", device_path_);
        return false;
    }
    
    logger->info("Using buffer type: {}", (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) ? "MULTIPLANAR" : "SINGLE-PLANAR");
    
    return true;
}


bool V4L2Capture::CheckSupportFormat()
{
    struct v4l2_fmtdesc fmtdesc; 
    fmtdesc.index=0; 
    fmtdesc.type=buffer_type_; 
    logger->info("Support format:");
    while(ioctl(fd_, VIDIOC_ENUM_FMT, &fmtdesc) != -1)
    {
        logger->info("\t{}.{}", fmtdesc.index + 1, reinterpret_cast<const char*>(fmtdesc.description));
        fmtdesc.index++;
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
    fmt_desc.type = buffer_type_;

    while (ioctl(fd_, VIDIOC_ENUM_FMT, &fmt_desc) == 0) {
        PixelFormat pf;
        pf.fourcc = fmt_desc.pixelformat;
        pf.description = reinterpret_cast<char*>(fmt_desc.description);
        formats.push_back(pf);
        fmt_desc.index++;
    }

    return formats;
}

bool V4L2Capture::SetFormat(uint32_t width, uint32_t height, uint32_t pixfmt) {
    if (!isOpened()) return false;

    v4l2_format fmt = {};
    fmt.type = buffer_type_;
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
    return true;
}

bool V4L2Capture::GetFormat(uint32_t& width, uint32_t& height, uint32_t& pixfmt) const {
    if (!isOpened()) return false;

    v4l2_format fmt = {};
    fmt.type = buffer_type_;

    if (ioctl(fd_, VIDIOC_G_FMT, &fmt) == -1) {
        return false;
    }

    width = fmt.fmt.pix.width;
    height = fmt.fmt.pix.height;
    pixfmt = fmt.fmt.pix.pixelformat;

    return true;
}

bool V4L2Capture::SetFrameRate(uint32_t fps) {
    if (!isOpened() || fps == 0) return false;

    v4l2_streamparm parm = {};
    parm.type = buffer_type_;

    // 首先尝试获取当前参数
    if (ioctl(fd_, VIDIOC_G_PARM, &parm) == -1) {
        // 如果获取失败，可能是设备不支持VIDIOC_G_PARM
        logger->warn("VIDIOC_G_PARM not supported, trying to set fps directly");
        
        // 直接设置参数，不检查当前状态
        parm.type = buffer_type_;
        
        if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            parm.parm.capture.timeperframe.numerator = 1;
            parm.parm.capture.timeperframe.denominator = fps;
        } else {
            parm.parm.capture.timeperframe.numerator = 1;
            parm.parm.capture.timeperframe.denominator = fps;
        }

        if (ioctl(fd_, VIDIOC_S_PARM, &parm) == -1) {
            logger->warn("VIDIOC_S_PARM also failed, device may not support dynamic fps setting");
            // 对于不支持动态帧率设置的设备，返回true表示忽略这个错误
            // 因为很多设备在设置格式时就已经确定了帧率
            return true;
        }
        
        return true;
    }

    // 如果获取成功，检查是否支持帧率设置
    bool supports_timeperframe = false;
    if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        supports_timeperframe = (parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME);
    } else {
        supports_timeperframe = (parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME);
    }

    if (!supports_timeperframe) {
        logger->warn("Device does not support dynamic fps configuration");
        return true;  // 返回true表示忽略这个错误
    }

    // 设置帧率 (fps = 1/timeperframe)
    if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        parm.parm.capture.timeperframe.numerator = 1;
        parm.parm.capture.timeperframe.denominator = fps;
    } else {
        parm.parm.capture.timeperframe.numerator = 1;
        parm.parm.capture.timeperframe.denominator = fps;
    }

    if (ioctl(fd_, VIDIOC_S_PARM, &parm) == -1) {
        logger->error("ioctl :VIDIOC_S_PARM fail.");
        return false;
    }
    
    logger->info("Frame rate set to {} fps", fps);
    return true;
}
bool V4L2Capture::StartStream(uint32_t buffer_count) {
    if (!isOpened() || is_streaming_) return false;
    
    // 将所有缓冲区加入队列
    for (uint32_t i = 0; i < buffer_list.size(); ++i) {
        if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            v4l2_buffer buf = {};
            v4l2_plane planes[VIDEO_MAX_PLANES] = {};
            
            buf.type = buffer_type_;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            buf.m.planes = planes;
            buf.length = VIDEO_MAX_PLANES;

            // 关键修复：正确初始化planes数组
            for (uint32_t j = 0; j < VIDEO_MAX_PLANES; ++j) {
                buf.m.planes[j].bytesused = 0;
                buf.m.planes[j].length = 0;
                buf.m.planes[j].data_offset = 0;
            }

            if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
                cleanupBuffers();
                logger->error("VIDIOC_QBUF failed for multi-planar: {}", strerror(errno));
                return false;
            }
        } else {
            v4l2_buffer buf = {};
            buf.type = buffer_type_;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
                cleanupBuffers();
                logger->error("VIDIOC_QBUF failed: {}", strerror(errno));
                return false;
            }
        }
    }

    // 开始流
    v4l2_buf_type type = buffer_type_;
    if (ioctl(fd_, VIDIOC_STREAMON, &type) == -1) {
        cleanupBuffers();
        logger->error("VIDIOC_STREAMON failed: {}", strerror(errno));
        return false;
    }

    is_streaming_ = true;
    logger->info("start stream ...");
    return true;
}
bool V4L2Capture::StopStream() {
    if (!is_streaming_) return true;

    v4l2_buf_type type = buffer_type_;
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

bool V4L2Capture::captureFrame(void*& image_data, size_t & size, uint8_t & index, timeval & timestamp, int timeout_ms) 
{
    if (!is_streaming_) {
        logger->error("captureFrame failed: stream is not started");
        return false;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);

    timeval tv;
    tv.tv_sec = timeout_ms / 1000;
    tv.tv_usec = (timeout_ms % 1000) * 1000;

    logger->debug("Waiting for frame with timeout: {}ms", timeout_ms);
    int r = select(fd_ + 1, &fds, nullptr, nullptr, &tv);
    
    if (r == 0) {
        logger->debug("select timeout: no frame available within {}ms", timeout_ms);
        return false; // 超时
    }
    if (r == -1) {
        logger->error("select error: {}", strerror(errno));
        return false; // 错误
    }

    logger->debug("select returned: {} (frame available)", r);

    // 处理多平面设备
    if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        v4l2_buffer buf = {};
        v4l2_plane planes[VIDEO_MAX_PLANES] = {};
        
        buf.type = buffer_type_;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.m.planes = planes;
        buf.length = VIDEO_MAX_PLANES;

        // 关键修复：正确初始化planes数组
        for (uint32_t i = 0; i < VIDEO_MAX_PLANES; ++i) {
            buf.m.planes[i].bytesused = 0;
            buf.m.planes[i].length = 0;
            buf.m.planes[i].data_offset = 0;
        }

        if (ioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
            logger->error("VIDIOC_DQBUF failed for multi-planar: {}", strerror(errno));
            return false;
        }

        logger->debug("Multi-planar VIDIOC_DQBUF success: index={}, plane0_bytes_used={}", 
                     buf.index, buf.m.planes[0].bytesused);

        if (buf.index >= buffer_list.size()) {
            logger->error("Buffer index out of range: {} >= {}", buf.index, buffer_list.size());
            ioctl(fd_, VIDIOC_QBUF, &buf);
            return false;
        }

        timestamp = buf.timestamp;
        image_data = buffer_list[buf.index].start;
        index = buf.index;
        size = buf.m.planes[0].bytesused; // 使用第一个平面的数据大小

    } else {
        // 单平面设备处理（原有逻辑）
        v4l2_buffer buf = {};
        buf.type = buffer_type_;
        buf.memory = V4L2_MEMORY_MMAP;

        if (ioctl(fd_, VIDIOC_DQBUF, &buf) == -1) {
            logger->error("VIDIOC_DQBUF failed: {}", strerror(errno));
            return false;
        }

        logger->debug("Single-planar VIDIOC_DQBUF success: index={}, bytes_used={}", 
                     buf.index, buf.bytesused);

        if (buf.index >= buffer_list.size()) {
            logger->error("Buffer index out of range: {} >= {}", buf.index, buffer_list.size());
            ioctl(fd_, VIDIOC_QBUF, &buf);
            return false;
        }

        timestamp = buf.timestamp;
        image_data = buffer_list[buf.index].start;
        index = buf.index;
        size = buf.bytesused;
    }

    logger->debug("Frame captured: index={}, size={} bytes, timestamp={}.{}", 
                 index, size, timestamp.tv_sec, timestamp.tv_usec);

    return true;
}

bool V4L2Capture::returnFrame(uint32_t index) {
    if (!is_streaming_) return false;

    if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
        v4l2_buffer buf = {};
        v4l2_plane planes[VIDEO_MAX_PLANES] = {};
        
        buf.type = buffer_type_;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;
        buf.m.planes = planes;
        buf.length = VIDEO_MAX_PLANES;

        // 关键修复：正确初始化planes数组
        for (uint32_t i = 0; i < VIDEO_MAX_PLANES; ++i) {
            buf.m.planes[i].bytesused = 0;
            buf.m.planes[i].length = 0;
            buf.m.planes[i].data_offset = 0;
        }

        if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
            logger->error("VIDIOC_QBUF failed for multi-planar: {}", strerror(errno));
            return false;
        }
    } else {
        v4l2_buffer buf = {};
        buf.type = buffer_type_;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = index;

        if (ioctl(fd_, VIDIOC_QBUF, &buf) == -1) {
            logger->error("VIDIOC_QBUF failed: {}", strerror(errno));
            return false;
        }
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
// ... existing code ...
bool V4L2Capture::InitBuffers(uint32_t buffer_count) {
    if (buffer_count < 2) return false;

    // 请求缓冲区
    v4l2_requestbuffers req = {};
    req.count = buffer_count;
    req.type = buffer_type_;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_REQBUFS, &req) == -1) {
        logger->error("VIDIOC_REQBUFS failed: {}", strerror(errno));
        return false;
    }

    if (req.count < 2) {
        logger->error("Requested {} buffers but got only {}", buffer_count, req.count);
        return false;
    }

    buffer_list.resize(req.count);

    // 映射缓冲区 - 根据设备类型分别处理
    for (uint32_t i = 0; i < req.count; ++i) {
        if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            // 多平面设备处理
            v4l2_buffer buf = {};
            v4l2_plane planes[VIDEO_MAX_PLANES] = {};
            
            buf.type = buffer_type_;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            buf.m.planes = planes;
            buf.length = VIDEO_MAX_PLANES;  // 先查询最大可能平面数

            if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
                logger->error("VIDIOC_QUERYBUF failed for multi-planar: {}", strerror(errno));
                cleanupBuffers();
                return false;
            }

            // 关键修复：使用设备实际返回的平面数量
            uint32_t actual_planes = buf.length;
            logger->info("Buffer {} has {} planes (NV12 should have 2)", i, actual_planes);

            // 只映射实际存在的平面
            for (uint32_t plane_idx = 0; plane_idx < actual_planes; ++plane_idx) {
                buffer_list[i].planes[plane_idx].start = mmap(nullptr, buf.m.planes[plane_idx].length,
                                                        PROT_READ | PROT_WRITE,
                                                        MAP_SHARED,
                                                        fd_, buf.m.planes[plane_idx].m.mem_offset);
                buffer_list[i].planes[plane_idx].length = buf.m.planes[plane_idx].length;
                buffer_list[i].planes[plane_idx].offset = buf.m.planes[plane_idx].data_offset;

                if (buffer_list[i].planes[plane_idx].start == MAP_FAILED) {
                    logger->error("mmap failed for buffer {} plane {}: {}", 
                                 i, plane_idx, strerror(errno));
                    cleanupBuffers();
                    return false;
                }

                logger->debug("Multi-planar buffer {} plane {} mapped: offset={}, length={}, data_offset={}", 
                             i, plane_idx, buf.m.planes[plane_idx].m.mem_offset, 
                             buf.m.planes[plane_idx].length, buf.m.planes[plane_idx].data_offset);
            }

            // 对于向后兼容，设置主缓冲区指针为第一个平面
            buffer_list[i].start = buffer_list[i].planes[0].start;
            buffer_list[i].length = buffer_list[i].planes[0].length;

        } else {
            // 单平面设备处理（原有逻辑）
            v4l2_buffer buf = {};
            buf.type = buffer_type_;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;

            if (ioctl(fd_, VIDIOC_QUERYBUF, &buf) == -1) {
                logger->error("VIDIOC_QUERYBUF failed: {}", strerror(errno));
                cleanupBuffers();
                return false;
            }

            buffer_list[i].start = mmap(nullptr, buf.length,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED,
                                    fd_, buf.m.offset);
            buffer_list[i].length = buf.length;
            buffer_list[i].index = i;

            // 单平面设备：第一个平面就是整个缓冲区
            buffer_list[i].planes[0].start = buffer_list[i].start;
            buffer_list[i].planes[0].length = buffer_list[i].length;
            buffer_list[i].planes[0].offset = 0;

            logger->debug("Single-planar buffer {} mapped: offset={}, length={}", 
                         i, buf.m.offset, buf.length);
        }

        if (buffer_list[i].start == MAP_FAILED) {
            logger->error("mmap failed for buffer {}: {}", i, strerror(errno));
            cleanupBuffers();
            return false;
        }
    }
    
    logger->info("Buffersssssss initialized: {} buffers, type={}", 
                req.count, 
                (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) ? "MULTIPLANAR" : "SINGLE-PLANAR");
    return true;
}

void V4L2Capture::cleanupBuffers() {
    for (auto& buffer : buffer_list) {
        if (buffer_type_ == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE) {
            // 多平面设备：释放所有平面
            for (uint32_t i = 0; i < VIDEO_MAX_PLANES; ++i) {
                if (buffer.planes[i].start != nullptr && buffer.planes[i].start != MAP_FAILED) {
                    munmap(buffer.planes[i].start, buffer.planes[i].length);
                    buffer.planes[i].start = nullptr;
                }
            }
        } else {
            // 单平面设备
            if (buffer.start != nullptr && buffer.start != MAP_FAILED) {
                munmap(buffer.start, buffer.length);
                buffer.start = nullptr;
            }
        }
    }
    buffer_list.clear();
}