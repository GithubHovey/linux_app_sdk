#ifndef RKCAPTURE_H
#define RKCAPTURE_H

#include "utils.h"
#include "rkmedia_api.h"
#ifdef RKAIQ
#define DEFAULT_IQFILE_PATH "/oem/etc/iqfiles"
#else
#define DEFAULT_IQFILE_PATH ""
#endif
class RKCapture {
public:
    RKCapture(std::string deviceName, std::string logfile, RK_S32 deviceId = 0, RK_S32 channelId = 0, std::string iq_files_path = DEFAULT_IQFILE_PATH, uint8_t buf_cnt = 10);
    ~RKCapture();

    int init(uint32_t width, uint32_t height, uint32_t fps, std::string outputFormat, uint32_t outputWidth, uint32_t outputHeight);
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
    RK_S32 deviceId;
    RK_S32 channelId;
    bool initialized;
    std::string iq_file_dir;
    MPP_CHN_S stSrcChn;
    MPP_CHN_S stDestChn;
    uint8_t current_mb_index = 0;
};

#endif // RKCAPTURE_H