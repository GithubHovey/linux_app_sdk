// Config.h
#pragma once
#include <yaml-cpp/yaml.h>
#include <string>
#include <vector>
#include <mutex>

class Config {
public:
    // 删除拷贝构造和赋值操作
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // 获取单例实例
    static Config& getInstance() {
        static Config instance;  // C++11 保证线程安全的局部静态变量
        return instance;
    }

    // 加载配置文件（需在程序启动时调用）
    bool load(const std::string& filepath) {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            config_ = YAML::LoadFile(filepath);
            return true;
        } catch (const YAML::Exception& e) {
            std::cerr << "Config Load Error: " << e.what() << std::endl;
            return false;
        }
    }

    // 获取配置项（模板方法，支持类型自动转换）
    template <typename T>
    T get(const std::string& key, const T& default_value = T()) const {
        std::lock_guard<std::mutex> lock(mutex_);
        try {
            return config_[key].as<T>();
        } catch (...) {
            return default_value;
        }
    }

    // 获取嵌套配置（如 "robot.joints.pid.p"）
    template <typename T>
    T getNested(const std::vector<std::string>& keys, const T& default_value = T()) const {
        std::lock_guard<std::mutex> lock(mutex_);
        YAML::Node node = config_;
        for (const auto& key : keys) {
            if (!node[key]) return default_value;
            node = node[key];
        }
        return node.as<T>();
    }

private:
    Config() = default;  // 私有构造函数
    YAML::Node config_;  // 存储解析后的YAML数据
    mutable std::mutex mutex_;  // 保证线程安全
};


/*-----------usage---------
// main.cpp
#include "yamlconfig.h"

int main() {
    // 加载配置文件（失败时退出）
    if (!Config::getInstance().load("robot_config.yaml")) {
        return 1;
    }

    // 启动其他模块...
    RobotController controller;
    controller.init();
    return 0;
}


// RobotController.cpp
#include "yamlconfig.h"

void RobotController::init() {
    // 直接通过单例获取配置
    double max_speed = Config::getInstance().get<double>("max_speed", 1.0);  // 带默认值

    // 获取嵌套配置（等效于 YAML 中的 robot.joints[0].pid.p）
    double pid_p = Config::getInstance().getNested<double>({"robot", "joints", "0", "pid", "p"});

    // 线程安全访问（无需额外处理）
    std::thread t1([&]() {
        bool enable = Config::getInstance().get<bool>("enable_sensors");
    });
    t1.join();
}


*/