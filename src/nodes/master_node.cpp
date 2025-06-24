#include "utils.h"
#include "yamlconfig.h"
#include "module_manager.h"
// #include "AIchat.h"
#include "vision.h"
#include <csignal>
volatile int QUIT_FLAG = 0;
//static char* TSDEV="/dev/input/event0";
void signal_handler(int sig) {
    std::cout << "get ctrl + c signal ,exit " << std::endl;
    QUIT_FLAG = 1;  // 设置退出标志
}
int main(int argc, char** argv)
{
    /*1. 配置文件解析*/
    if (argc != 2) {
        std::cerr << "Error: YAML config file path is required!\n";
        std::cerr << "Usage: " << argv[0] << " <path/to/config.yaml>\n";
        std::cerr << "Example: " << argv[0] << " /opt/robot_config.yaml\n";
        return EXIT_FAILURE;
    }
    if (!Config::getInstance().load(argv[1])) {
        std::cerr << "Failed to load config file: " << argv[1] << "\n";
        return EXIT_FAILURE;
    }
    /*2.日志系统初始化*/ 
    spdlog::set_pattern("[%H:%M:%S] [%n] [%l] %v");  // 设置日志格式
    spdlog::flush_every(std::chrono::seconds(3)); //每3s写入一次日志
    // std::shared_ptr<spdlog::logger> main_logger = spdlog::basic_logger_mt("main_logger", "logs/main.log");

    /*3.模块初始化*/ 
    ModuleManager module_manager;
    // module_manager.addModule(std::make_shared<AIchat>
    //     ("aichat", 16000, 1, 40, "./config/snowboy/common.res", "./config/snowboy/snowboy.umdl"));
    module_manager.addModule(std::make_shared<Vision>("vision center"));
    module_manager.startAll();
    while (!QUIT_FLAG) {
        // if (getExitSignal()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    module_manager.stopAll();

    return 0;
}


