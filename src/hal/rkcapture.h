#ifndef RKCAPTURE_H
#define RKCAPTURE_H

#include "utils.h"
#include "rkmedia_api.h"

class RKCapture {
public:
    RKCapture(const std::string& deviceName, std::string logfile, int deviceId = 0, uint8_t buf_cnt = 10);
    ~RKCapture();

    int init(int width, int height, int fps, 
              IMAGE_TYPE_E outputFormat, 
              int outputWidth, int outputHeight);
    int StartStream(void);
    int captureFrame(void*& image_data, size_t & size, uint8_t & index, timeval & timestamp, int timeout = 0);
    
    // Add new API for returning buffers
    int returnFrame(uint8_t index);
    int StopStream();
    int close();
private:
    // struct rkcapture_buffer
    // {
    //     uint8_t index;
    //     MEDIA_BUFFER mb;

    // };
    std::vector<MEDIA_BUFFER> mb_list;
    std::string deviceName;
    int deviceId;
    bool initialized;
};

#endif // RKCAPTURE_H