#pragma once
#include "module.h"
#include "camera.h"
class Vision : public Module {
public:
    // struct Unit {  // 嵌套在Vision类中的结构体
    //         Camera camera; 
    //         std::vector<int> resolution;
    //         int fps;
    //         bool record_processed;
    //         // 其他相机参数...
    //     };
    Vision(const std::string& name):
    Module(name)
    {

    }
    ~Vision()override{

    };
    bool load_from_config() override ;
    bool init() override ;
private:
    std::vector<std::unique_ptr<Camera>> camera_list;
    void capture_thread();
    void tradition_cv_thread();
    void yolo_detection_thread();
    void record_thread();
protected:
    // 模块线程函数
    void moduleThreadFunc() override ;
 
};