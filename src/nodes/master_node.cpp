#include "utils.h"
#include "module_manager.h"
#include "AIchat.h"
#include <csignal>
volatile int QUIT_FLAG = 0;
//static char* TSDEV="/dev/input/event0";
void signal_handler(int sig) {
    std::cout << "get ctrl + c signal ,exit " << std::endl;
    QUIT_FLAG = 1;  // 设置退出标志
}
int main()
{
    std::shared_ptr<spdlog::logger> main_logger = spdlog::basic_logger_mt("main_logger", "logs/main.log");
    ModuleManager fangfang_module_manager;
    fangfang_module_manager.addModule(std::make_shared<AIchat>
        ("aichat", 16000, 1, 40, "./config/snowboy/common.res", "./config/snowboy/common.pdml"));
    fangfang_module_manager.startAll();
    while (!QUIT_FLAG) {
        // if (getExitSignal()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    fangfang_module_manager.stopAll();

    return 0;
}


