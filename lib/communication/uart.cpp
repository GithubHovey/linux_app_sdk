// uart.cpp
#include "uart.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <cstring>
#include <iostream>
UARTDriver::UARTDriver(size_t rx_array_size) : fd(-1), baudrate(B115200), parity('N'),
                                                stop_bits(1), data_bits_(8),
                                                hardware_flow(false), software_flow(false),
                                                rx_array(rx_array_size) {}

UARTDriver::~UARTDriver() {
    close();
}

bool UARTDriver::open(const std::string& port, int baudrate, bool non_blocking) {
    if (isOpen()) {
        close();
    }

    this->port = port;
    this->baudrate = baudrate;

    fd = ::open(this->port.c_str(), O_RDWR | O_NOCTTY | (non_blocking ? O_NONBLOCK : 0));
    if (fd == -1) {
        return false;
    }

    struct termios options;
    if (tcgetattr(fd, &options) != 0) {
        return false;
    }

    // 设置输入输出波特率
    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);

    // 设置数据位
    options.c_cflag &= ~CSIZE;
    switch (data_bits_) {
        case 5: options.c_cflag |= CS5; break;
        case 6: options.c_cflag |= CS6; break;
        case 7: options.c_cflag |= CS7; break;
        case 8: 
        default: options.c_cflag |= CS8; break;
    }

    // 设置校验位
    switch (parity) {
        case 'O': case 'o':
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        case 'E': case 'e':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        case 'N': case 'n':
        default:
            options.c_cflag &= ~PARENB;
            break;
    }

    // 设置停止位
    if (stop_bits == 2) {
        options.c_cflag |= CSTOPB;
    } else {
        options.c_cflag &= ~CSTOPB;
    }

    // 设置硬件流控
    if (hardware_flow) {
        options.c_cflag |= CRTSCTS;
    } else {
        options.c_cflag &= ~CRTSCTS;
    }

    // 设置软件流控
    if (software_flow) {
        options.c_iflag |= IXON | IXOFF | IXANY;
    } else {
        options.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    // 设置原始输入模式
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= ~OPOST;

    // 设置超时和最小读取字符数
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 10; // 1秒超时

    // 应用设置
    if (tcsetattr(fd, TCSANOW, &options) != 0) {
        return false;
    }

    // 刷新输入输出缓冲区
    tcflush(fd, TCIOFLUSH);
    return true;
}

void UARTDriver::close() {
    if (fd != -1) {
        ::close(fd);
        fd = -1;
    }
}

bool UARTDriver::isOpen() const {
    return fd != -1;
}

bool UARTDriver::dataAvailable(unsigned int timeout_ms) {
    if (!isOpen()) {
        std::cerr << "UART not opened" << std::endl;
        return false;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;
    
    int ret = select(fd + 1, &read_fds, nullptr, nullptr, &timeout);
    
    if (ret == -1) {
        std::cerr << "select error: " << strerror(errno) << std::endl;
        return false;
    } else if (ret == 0) {
        // std::cout << "select timeout" << std::endl;
        return false;
    }

    if (!FD_ISSET(fd, &read_fds)) {
        std::cerr << "FD not set in read_fds" << std::endl;
        return false;
    }

    return true;
}
// 读取数据到uart环形缓冲区
int UARTDriver::read() {
    if (!isOpen()) return -UART_OPEN_FAILED;

    std::lock_guard<std::mutex> lock(rx_array.mutex);
tohead:
    // 计算可写入空间
    size_t free_space = rx_array.buffer.size() - rx_array.write_index; 
    if (free_space == 0) {
        // write pos 回绕到0位置
        rx_array.write_index = 0;
        free_space = rx_array.buffer.size();
    }
    // if(rx_array.unused_count > 0 && rx_array.write_index==read_index) //缓冲区满
    if(rx_array.unused_count >= rx_array.buffer.size())
    {
        std::cout << "buffer overflow! unused:" << rx_array.unused_count << "buf size:" << rx_array.buffer.size() << std::endl;
        return -UART_RX_BUFFER_OVERFLOW;
    }
    if(rx_array.write_index < rx_array.read_index) //衔尾态
    {
        free_space = rx_array.read_index - rx_array.write_index; //写入区域不能将超过read_pos，避免衔尾蛇效应
    }
    // 直接读取到rx_array
    ssize_t bytes_read = ::read(fd, rx_array.buffer.data() + rx_array.write_index, free_space);
    
    if (bytes_read < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return UART_OK; // 非阻塞模式下没有数据可读
        }
        return -UART_READ_FAILED; // 读取错误
    }
    if(bytes_read == 0)
    {
        return UART_OK;
    }
    std::cout << "bytes_read:+" << bytes_read << std::endl;
    // 环形缓冲区处理逻辑
    size_t tar_write_pos = rx_array.write_index + bytes_read;
    if(tar_write_pos > rx_array.buffer.size())//
    {
        std::cout << "unknown error:" << tar_write_pos << std::endl;
        return -UART_UNKNOWN_ERROR; //一般不会进这里，属于防御编程，进来了一定是代码BUG,验证长期不进入可以删除
    }
    rx_array.write_index = tar_write_pos;
    rx_array.unused_count += bytes_read;
    std::cout << "unused_count:" << rx_array.unused_count << std::endl;
    // for(auto it:rx_array.buffer)
    // {
    //     std::cout << std::hex << " "<< (int)it;
    // }
    if(rx_array.buffer.size() == rx_array.write_index)
    {
        goto tohead; //可能没读满，再读一次
    }
    return UART_OK;
}

int UARTDriver::write(const std::vector<uint8_t>& data) {
    if (!isOpen()) return -1;

    ssize_t written = ::write(fd, data.data(), data.size());
    if (written < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // 非阻塞模式下无法立即写入
        }
        return -1;
    }

    return static_cast<int>(written);
}
// 消费数据后调用此方法更新读指针和剩余数据量
CircularArray<uint8_t> & UARTDriver::getRxArray()
{
    return rx_array;
}