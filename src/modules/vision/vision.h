#pragma once
#include "module.h"
#include "portaudio_driver.h"
#include "snowboy-detect-c-wrapper.h"
// void AIchat_thread();
class Vision : public Module {
public:
    Vision(const std::string& name, int sample_rate, int channels, int frame_duration,
    const std::string& resource_filename,const std::string& model_str
    ):
    Module(name),
    port_audio_driver(sample_rate, channels, frame_duration),
    resource_filename(resource_filename),
    model_str(model_str)
    {
        detector = SnowboyDetectConstructor(this->resource_filename.c_str(),this->model_str.c_str());
    }
    ~Vision()override{
        SnowboyDetectDestructor(detector);
    };
    bool init() override ;
protected:
    // 模块线程函数
    void moduleThreadFunc() override ;
};