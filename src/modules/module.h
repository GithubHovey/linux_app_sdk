#pragma once
// #ifdef AICHAT_MODULE_ENABLE

// #endif

#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <condition_variable>

class Module {
public:
    Module();
    virtual ~Module();

    // 初始化模块
    virtual bool init();

    // // 启动模块线程
    // void start();

    // // 停止模块线程
    // void stop();

    // // 获取模块名称
    // std::string getName() const;

protected:
    // 模块线程函数（子类需要实现）
    virtual void moduleThreadFunc();
    volatile uint8_t quit_flag;
    // 模块名称
    std::string name;

    // 线程控制
    std::thread thread;
    std::atomic<bool> running;

    // 互斥锁和条件变量（可选，根据需要使用）
    std::mutex mutex;
    std::condition_variable cv;
};