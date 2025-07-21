#pragma once
// #ifdef AICHAT_Driver_ENABLE

// #endif
#include "utils.h"
#include "yamlconfig.h"
#include <atomic>
#include <condition_variable>


class Driver {
public:
    Driver(const std::string& name):
    name(name)
    {
        logger = LogManager::GetLogger(name);
        // logfile = "logs/" + name + ".log";
        // logger = spdlog::basic_logger_mt(name, logfile);
        // logger->set_level(spdlog::level::debug);  // 允许info及以上级别
        logger->info("---------------");
        logger->info("Driver initing...");
    }
    virtual ~Driver(){}

    // 实例化模块类
    virtual bool load_from_config() = 0;

    // // 获取模块名称

protected:
    volatile uint8_t quit_flag;
    // 模块名称
    std::string name;
    // 日志记录器
    std::shared_ptr<spdlog::logger> logger;
    // 线程

    std::mutex mutex;
    std::string logfile;
    // 模块线程函数（子类需要实现）

};