#pragma once
// #ifdef AICHAT_MODULE_ENABLE

// #endif
#include "utils.h"
#include <atomic>
#include <string>
#include <condition_variable>

class Module {
public:
    Module(const std::string& name):
    name(name)
    {
        logger = spdlog::basic_logger_mt(name, "logs/" + name + ".log");
        logger->set_level(spdlog::level::debug);  // 允许info及以上级别
        logger->info("---------------");
        logger->info("module initing...");
    }
    virtual ~Module(){}

    // 初始化模块
    virtual bool init();

    // 启动模块线程
    virtual void start();

    // 停止模块线程
    virtual void stop();

    // // 获取模块名称
    // std::string getName() const;

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
    // 模块线程函数（子类需要实现）
    virtual void moduleThreadFunc();
 private:   
    void startThread();
    void stopThread();
};