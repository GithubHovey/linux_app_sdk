#pragma once
// #ifdef AICHAT_MODULE_ENABLE

// #endif
#include "utils.h"
#include "yamlconfig.h"
#include <atomic>
#include <condition_variable>


class Module {
public:
    Module(const std::string& name):
    name(name)
    {
        logfile = "logs/" + name + ".log";
        logger = spdlog::basic_logger_mt(name, logfile);
        logger->set_level(spdlog::level::debug);  // 允许info及以上级别
        logger->info("---------------");
        logger->info("module initing...");
    }
    virtual ~Module(){}

    // 实例化模块类
    virtual bool load_from_config() = 0;
    // 初始化模块
    virtual bool init() = 0;

    // 启动模块线程
    void start();

    // 停止模块线程
    void stop();

    // // 获取模块名称

protected:
    volatile uint8_t quit_flag;
    // 模块名称
    std::string name;
    // 日志记录器
    std::shared_ptr<spdlog::logger> logger;
    // 线程
    std::thread thread;
    std::mutex mutex;
    std::atomic<bool> _thread_running{false};
    std::string logfile;
    // 模块线程函数（子类需要实现）
    virtual void moduleThreadFunc() = 0;
 private:   
    void startThread();
    void stopThread();
};