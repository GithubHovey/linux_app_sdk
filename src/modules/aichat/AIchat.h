#pragma once
#include "module.h"
#include "portaudio_driver.h"
#include "snowboy-detect-c-wrapper.h"
// void AIchat_thread();
class AIchat : public Module {
public:
    AIchat(const std::string& name, int sample_rate, int channels, int frame_duration,
    const std::string& resource_filename,const std::string& model_str
    ):
    Module(name),
    port_audio_driver(sample_rate, channels, frame_duration),
    resource_filename(resource_filename),
    model_str(model_str)
    {
        detector = SnowboyDetectConstructor(this->resource_filename.c_str(),this->model_str.c_str());
    }
    ~AIchat()override{
        SnowboyDetectDestructor(detector);
    };
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
    SnowboyDetect* detector;
    // Snowboy 语音检测器
    // snowboy::SnowboyDetect snowboy_detector;
    // 语音检测标志
    // bool hotwordDetected;
};
