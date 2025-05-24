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
    ModuleManager fangfang_module_manager;
    fangfang_module_manager.addModule(std::make_shared<AIchat>
        (16000, 1, 40, "third_party/snowboy/resources/common.res", "third_party/snowboy/resources/common.pdml"));
    fangfang_module_manager.startAll();
    while (!QUIT_FLAG) {
        // if (getExitSignal()) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    fangfang_module_manager.stopAll();
    // int sample_rate = 16000;
    // int channels = 1;
    // int frame_duration = 40;
    // AIchat *fangfang_ai = new AIchat(sample_rate, channels, frame_duration,
    // "third_party/snowboy/resources/common.res","third_party/snowboy/resources/common.pdml" );
    // try {
    //     std::thread t1(AIchat_thread);
    //     // std::thread t2(gui_thread);

    //     t1.join();
    //     // t2.join();
    // } catch (const std::exception& e) {
    //     std::cerr << "Fatal error: " << e.what() << std::endl;
    //     return EXIT_FAILURE;
    // }
    return 0;
}


