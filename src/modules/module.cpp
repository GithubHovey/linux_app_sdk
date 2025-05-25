#include "module.h"

bool Module::init(){
    return true;
}
void Module::start(){
    startThread();
}
void Module::stop(){
    stopThread(); 
}
void Module::moduleThreadFunc(){

}

void Module::startThread() {
        if (!_thread_running) {
            _thread_running = true;
            thread = std::thread(&Module::moduleThreadFunc, this);
        }
    }

void Module::stopThread() {
    if (_thread_running) {
        _thread_running = false;
        if (thread.joinable()) thread.join();
    }
}
