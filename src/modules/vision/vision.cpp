#include "vision.h"
bool Vision::load_from_config() {
    auto config = Config::getInstance().get<YAML::Node>("vision");
    if (!config["camera_list"]) return false;

    camera_list.clear();
    for (const auto& cam_entry : config["camera_list"]) {
        const auto& cam_config = cam_entry.second;
        
        if (cam_config["enable"].as<bool>()) {
            auto camera = std::make_unique<Camera>(
                cam_config["device"].as<std::string>(),
                cam_config["width"].as<uint32_t>(),
                cam_config["height"].as<uint32_t>(),
                cam_config["fps"].as<uint32_t>()
            );           

            camera_list.push_back(std::move(camera));
        } else {
            camera_list.push_back(nullptr);
        }
    }
    return true;
}

bool Vision::init()
{
    for(auto& camera : camera_list)
    {
        if(camera) {                // 检查指针有效性
            int ret = camera->init();
            if(ret != 0) return false;
        }
    }
    return true;
}

void Vision::moduleThreadFunc()
{

}

void Vision::capture_thread()
{

}

void Vision::tradition_cv_thread()
{

}

void Vision::yolo_detection_thread()
{

}

void Vision::record_thread()
{

}