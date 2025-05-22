#include "robot_thread.h"
#include <csignal>
volatile int QUIT_FLAG = 0;
//static char* TSDEV="/dev/input/event0";
void signal_handler(int sig) {
    std::cout << "get ctrl + c signal ,exit " << std::endl;
    QUIT_FLAG = 1;  // 设置退出标志
}
int main()
{
    try {
        std::thread t1(AIchat_thread);
        // std::thread t2(gui_thread);

        t1.join();
        // t2.join();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}


