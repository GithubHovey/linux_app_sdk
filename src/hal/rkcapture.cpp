#include "rkcapture.h"

RKCapture::RKCapture(const std::string& deviceName, std::string logfile, int deviceId, uint8_t buf_cnt)
    : deviceName(deviceName), deviceId(deviceId), initialized(false) {
    mb_list.resize(buf_cnt);

}

RKCapture::~RKCapture() {
    // Destructor implementation
}

int RKCapture::init(int width, int height, int fps, IMAGE_TYPE_E outputFormat, int outputWidth, int outputHeight) {

    return 0;
}
int RKCapture::StartStream(void)
{
    return 0;
}
int RKCapture::captureFrame(void*& image_data, size_t & size, uint8_t & index, timeval & timestamp, int timeout) {
    // TODO: Implementation
    return 0;
}

int RKCapture::returnFrame(uint8_t index) {
    // RK_MPI_MB_ReleaseBuffer(rk_buffer);
    return 0;
}
int RKCapture::StopStream()
{
    return 0;
}
int RKCapture::close()
{
    return 0;
}