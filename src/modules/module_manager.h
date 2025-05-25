#pragma once
#include "module.h"
class ModuleManager {
public:
    void addModule(std::shared_ptr<Module> module) {
        this->module_list.push_back(module);
    }

    void startAll() {
        for (auto& module : this->module_list) {
            module->init();  // 内部调用 startThread()
            module->start();
        }
    }

    void stopAll() {
        for (auto& module : this->module_list) {
            module->stop();
        }
    }

private:
    std::vector<std::shared_ptr<Module>> module_list;
};