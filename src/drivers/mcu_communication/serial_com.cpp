// #include "utils.h"
#include "utils.h"
#include "uart.h"
#include "mcu_protocol.h"
#define UART_BUFFER_SIZE    20
extern volatile int QUIT_FLAG;
void serial_com_thread(RingBuffer<MCU_cmd>& rx_ring, RingBuffer<MCU_cmd>& tx_ring) {
    UARTDriver uart(UART_BUFFER_SIZE);
    if (!uart.open("/dev/ttyS3", B115200, true)) { // 非阻塞模式
        std::cerr << "Failed to open UART port" << std::endl;
        return;
    }

    std::vector<uint8_t> rx_buffer;
    MCU_cmd rx_cmd;

    std::cout << "communication_thread running..." << std::endl;

    while (!QUIT_FLAG) {
        MCUProtocol protocol;
        // 接收 UART 数据
        if (uart.dataAvailable(0)) { // 等待数据可用
            if(uart.read() == UART_OK)
            {
                MCU_cmd cmd;
                while (1)
                {
                    if(protocol.unpack(uart.getRxArray(),cmd))
                    {
                        rx_ring.push(cmd);
                    }else{
                        break;
                    }
                }
            }
        }
        // 从发送 RingBuffer 中读取数据
        if (tx_ring.size() > 0) {
            if (tx_ring.pop(rx_cmd)) {
                // 封包数据
                std::vector<uint8_t> tx_buffer = protocol.pack(rx_cmd);

                // 发送数据
                int bytes_written = uart.write(tx_buffer);
                if (bytes_written != static_cast<int>(tx_buffer.size())) {
                    std::cerr << "Failed to write all bytes to UART" << std::endl;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // 避免 CPU 占用过高
    }

    uart.close();
}
