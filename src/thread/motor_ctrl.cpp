#include "robot_thread.h"
extern volatile int QUIT_FLAG;
void motor_thread() {
    // pthread_setname_np(pthread_self(), "skg_com");
    std::cout << "communication_thread running..." << std::endl;
    while (!QUIT_FLAG) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
