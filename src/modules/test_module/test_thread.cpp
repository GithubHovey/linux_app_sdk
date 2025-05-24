#include "utils.h"
#include "uart.h"
#include "ringbuffer.h"
#include "mcu_protocol.h"

extern volatile int QUIT_FLAG;
void test_thread(RingBuffer<MCU_cmd>& rx_buf, RingBuffer<MCU_cmd>& tx_buf) {
    while(!QUIT_FLAG) {
        // 准备发送的数据
        MCU_cmd tx_cmd{};
        tx_cmd.device_type = 1;  // 设备类型
        tx_cmd.device_id = 2;    // 设备ID
        tx_cmd.is_data = 1;      // 数据帧
        tx_cmd.type = 3;         // 数据类型
        tx_cmd.data1 = 0xAB;     // 数据1
        tx_cmd.data2 = 0xCD;     // 数据2
        
        // 发送数据
        // if (tx_buf.push(tx_cmd)) {
        //     std::cout << "Sent cmd: type=" << (int)tx_cmd.device_type 
        //               << " id=" << (int)tx_cmd.device_id 
        //               << " data=" << (int)tx_cmd.data1 << "," << (int)tx_cmd.data2 << std::endl;
        // }

        // 接收数据
        MCU_cmd rx_cmd;
        if (rx_buf.pop(rx_cmd)) {
            std::cout << "Received cmd: type=" << (int)rx_cmd.device_type
                      << " id=" << (int)rx_cmd.device_id
                      << " data=" << (int)rx_cmd.data1 << "," << (int)rx_cmd.data2 << std::endl;
            std::cout << std::endl;
        }
    }
        // 适当延迟
}
