#include "utils.h"
#include "yamlconfig.h"
#include "module_manager.h"
// #include "AIchat.h"
// #include "vision.h"
#include <csignal>
#include "log/logmanager.h"
#include <unistd.h>
volatile int QUIT_FLAG = 0;
//static char* TSDEV="/dev/input/event0";
void signal_handler(int sig) {
    std::cout << "get ctrl + c signal ,exit " << std::endl;
    QUIT_FLAG = 1;  // 设置退出标志
}
int main(int argc, char** argv)
{
    int opt;
    std::string configPath;
    std::string logConfigPath;
    while ((opt = getopt(argc, argv, "c:l:")) != -1) {
        switch (opt) {
            case 'c':
                configPath = optarg;
                break;
            case 'l':
                logConfigPath = optarg;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -c <config.yaml> [-l <logconf.yaml>]\n";
                std::cerr << "Example: " << argv[0] << " -c /opt/robot_config.yaml -l /opt/logconf.yaml\n";
                return EXIT_FAILURE;
        }
    }
        // 检查必须的参数
    if (configPath.empty()) {
        std::cerr << "Error: Config file path is required!\n";
        std::cerr << "Usage: " << argv[0] << " -c <config.yaml> [-l <logconf.yaml>]\n";
        return EXIT_FAILURE;
    }
    
    /*1. 配置文件解析*/
    if (!Config::getInstance().load(configPath)) {
        std::cerr << "Failed to load config file: " << configPath << "\n";
        return EXIT_FAILURE;
    }

    /*2.日志系统初始化*/ 
    try {
        // 初始化日志系统
        LogManager::Initialize(logConfigPath);
        
        // 设置全局异常处理器
        spdlog::set_error_handler([](const std::string& msg) {
            std::cerr << "Log error: " << msg << std::endl;
        });
        
        // 程序结束时

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    /*3.模块初始化*/ 
    ModuleManager module_manager;
    // module_manager.addModule(std::make_shared<AIchat>
    //     ("aichat", 16000, 1, 40, "./config/snowboy/common.res", "./config/snowboy/snowboy.umdl"));
    // module_manager.addModule(std::make_shared<Vision>("vision"));
    module_manager.startAll();
    while (!QUIT_FLAG) {
        // if (getExitSignal()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    module_manager.stopAll();
    LogManager::Shutdown();
    return 0;
}


