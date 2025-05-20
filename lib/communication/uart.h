#ifndef UART_DRIVER_H
#define UART_DRIVER_H

#include <string>
#include <vector>
#include <termios.h>
#include <sys/select.h>
#include <mutex>
#include "ringbuffer.h"
enum UARTERROR {
    UART_OK = 0,
    UART_OPEN_FAILED,
    UART_CONFIGURE_FAILED,
    UART_WRITE_FAILED,
    UART_READ_FAILED,
    UART_RX_BUFFER_OVERFLOW,
    UART_TX_BUFFER_OVERFLOW,
    UART_UNKNOWN_ERROR
};
class UARTDriver {
public:
    UARTDriver(size_t rx_buffer_size);
    ~UARTDriver();
    
    bool open(const std::string& port, int baudrate = B115200, bool non_blocking = true);
    void close();
    bool isOpen() const;
    
    // 非阻塞写入
    int write(const std::vector<uint8_t>& data);
    
    // 非阻塞读取
    int read();
    void consume(size_t bytes);
    // 检查是否有数据可读（非阻塞）
    bool dataAvailable(unsigned int timeout_ms = 0);
    // 在public部分添加
    CircularArray<uint8_t>& getRxArray();
    // bool setBaudrate(int baudrate);
    // bool setParity(char parity);
    // bool setStopBits(int stop_bits);
    // bool setDataBits(int data_bits);
    // bool setFlowControl(bool hardware, bool software);

private:
    // bool configurePort();
    
    int fd;
    std::string port;
    int baudrate;
    char parity;
    int stop_bits;
    int data_bits_;
    bool hardware_flow;
    bool software_flow;
    CircularArray<uint8_t> rx_array;

};

#endif // UART_DRIVER_H