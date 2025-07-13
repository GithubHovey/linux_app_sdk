#include "rkcapture.h"
#include "sample_common.h"
RKCapture::RKCapture(std::string deviceName, std::string logfile, RK_S32 deviceId, RK_S32 channelId, std::string iq_files_path, uint8_t buf_cnt)
    : deviceName(deviceName), deviceId(deviceId), channelId(channelId), iq_file_dir(iq_files_path), initialized(false) {
    mb_list.resize(buf_cnt);
    
}

RKCapture::~RKCapture() {
    // Destructor implementation
}

int RKCapture::init(uint32_t width, uint32_t height, uint32_t fps, std::string outputFormat, uint32_t outputWidth, uint32_t outputHeight) {


    #ifdef RKAIQ
        rk_aiq_working_mode_t hdr_mode = RK_AIQ_WORKING_MODE_NORMAL;
        RK_BOOL bMultictx = RK_FALSE;
        SAMPLE_COMM_ISP_Init(deviceId, hdr_mode, bMultictx, iq_file_dir.c_str());
        SAMPLE_COMM_ISP_Run(deviceId);
        SAMPLE_COMM_ISP_SetFrameRate(deviceId, fps);
    #endif
    RK_MPI_SYS_Init();
    VI_CHN_ATTR_S vi_chn_attr;
    vi_chn_attr.pcVideoNode = deviceName.c_str();
    vi_chn_attr.u32BufCnt = mb_list.size();
    vi_chn_attr.u32Width = width;
    vi_chn_attr.u32Height = height;
    vi_chn_attr.enPixFmt = IMAGE_TYPE_NV12;
    vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
    int ret = RK_MPI_VI_SetChnAttr(deviceId, channelId, &vi_chn_attr);
    
    RGA_ATTR_S stRgaAttr;
    stRgaAttr.bEnBufPool = RK_TRUE;
    stRgaAttr.u16BufPoolCnt = mb_list.size();
    stRgaAttr.u16Rotaion = 0;
    stRgaAttr.stImgIn.u32X = 0;
    stRgaAttr.stImgIn.u32Y = 0;
    stRgaAttr.stImgIn.imgType = IMAGE_TYPE_NV12;
    stRgaAttr.stImgIn.u32Width = width;
    stRgaAttr.stImgIn.u32Height = height;
    stRgaAttr.stImgIn.u32HorStride = width;
    stRgaAttr.stImgIn.u32VirStride = height;
    stRgaAttr.stImgOut.u32X = 0;
    stRgaAttr.stImgOut.u32Y = 0;
    // 
    if(outputFormat == std::string("rgb888"))
    {
        stRgaAttr.stImgOut.imgType = IMAGE_TYPE_BGR888;
    }else{
        stRgaAttr.stImgOut.imgType = IMAGE_TYPE_NV12;
    }    
    stRgaAttr.stImgOut.u32Width = outputWidth;
    stRgaAttr.stImgOut.u32Height = outputHeight;
    stRgaAttr.stImgOut.u32HorStride = outputWidth;
    stRgaAttr.stImgOut.u32VirStride = outputHeight;
    ret = RK_MPI_RGA_CreateChn(0, &stRgaAttr);
    if (ret) {
        printf("ERROR: Create rga[0] falied! ret=%d\n", ret);
        return -1;
    }

    return 0;
}
int RKCapture::StartStream(void)
{
    int ret = 0;
    ret |= RK_MPI_VI_EnableChn(deviceId, 0);
    if (ret) {
        printf("ERROR: Create vi[0] failed! ret=%d\n", ret);
        return -1;
    }
    MPP_CHN_S stSrcChn;
    stSrcChn.enModId = RK_ID_VI;
    stSrcChn.s32DevId = deviceId;
    stSrcChn.s32ChnId = channelId;
    MPP_CHN_S stDestChn;
    stDestChn.enModId = RK_ID_RGA;
    stDestChn.s32DevId = deviceId;
    stDestChn.s32ChnId = channelId;
    ret = RK_MPI_SYS_Bind(&stSrcChn, &stDestChn);
    if (ret) {
        printf("ERROR: Bind vi[0] and rga[0] failed! ret=%d\n", ret);
        return -1;
    }
    return 0;
}
int RKCapture::captureFrame(void*& image_data, size_t & size, uint8_t & index, timeval & timestamp, int timeout) {
    mb_list[current_mb_index] = RK_MPI_SYS_GetMediaBuffer(RK_ID_RGA, 0, -1);
    if (!mb_list[current_mb_index]) {
      printf("RK_MPI_SYS_GetMediaBuffer get null buffer!\n");
      return -1;
    }
    image_data = RK_MPI_MB_GetPtr(mb_list[current_mb_index]);
    size = RK_MPI_MB_GetSize(mb_list[current_mb_index]);
    uint64_t ts_us = RK_MPI_MB_GetTimestamp(mb_list[current_mb_index]);
    timestamp.tv_sec = ts_us / 1000000;
    timestamp.tv_usec = ts_us % 1000000;
    index = current_mb_index;
    if(++current_mb_index >= mb_list.size())
    {
        current_mb_index = 0;
    }
    return 0;
}

int RKCapture::returnFrame(uint8_t index) {
    RK_MPI_MB_ReleaseBuffer(mb_list[index]);
    return 0;
}
int RKCapture::StopStream()
{
    RK_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
    RK_MPI_RGA_DestroyChn(0);
    RK_MPI_VI_DisableChn(deviceId, 1);

    
    #ifdef RKAIQ
        SAMPLE_COMM_ISP_Stop(deviceId);
    #endif
    return 0;
}
int RKCapture::close()
{
    return 0;
}