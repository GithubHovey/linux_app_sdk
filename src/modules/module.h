#pragma once
// #ifdef AICHAT_MODULE_ENABLE

// #endif
#include "utils.h"
#include <atomic>
#include <string>
#include <condition_variable>

class Module {
public:
    Module();
    virtual ~Module();

    // 初始化模块
    virtual bool init();

    // 启动模块线程
    virtual void start(){
        startThread();
    }

    // 停止模块线程
    virtual void stop(){
        stopThread(); 
    }

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

    // 互斥锁和条件变量（可选，根据需要使用）
    std::mutex mutex;
    // std::condition_variable cv;
    std::atomic<bool> _thread_running{false};

    void startThread();

    void stopThread();
};