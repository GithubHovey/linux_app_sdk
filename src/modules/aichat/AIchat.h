#pragma once
#include "module.h"
#include "portaudio_driver.h"
#include "snowboy-detect-c-wrapper.h"
// void AIchat_thread();
class AIchat : public Module {
public:
    AIchat(int sample_rate, int channels, int frame_duration,
    const std::string& resource_filename,const std::string& model_str
    ):
    port_audio_driver(sample_rate, channels, frame_duration),
    resource_filename(resource_filename),
    model_str(model_str)
    {

    }
    ~AIchat()override{};
    bool init() override ;
protected:
    // 模块线程函数
    void moduleThreadFunc() override ;

    // 语音检测回调函数
    // void onHotwordDetected();

    // 音频处理逻辑
    // void processAudio();

    // PortAudio 驱动
    PortAudioDriver port_audio_driver;
    std::string resource_filename;
    std::string model_str;
    // Snowboy 语音检测器
    // snowboy::SnowboyDetect snowboy_detector;
    // 语音检测标志
    // bool hotwordDetected;
};
