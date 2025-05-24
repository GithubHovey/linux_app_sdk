#include "mcu_protocol.h"
#include <algorithm>
#include <string>
#include <cstring>
#include <iostream>
// 协议帧头定义
static const uint8_t FRAME_HEADER[] = {0xAA, 0x55};
static const size_t FRAME_HEADER_LEN = sizeof(FRAME_HEADER);

std::vector<uint8_t> MCUProtocol::pack(const MCU_cmd& cmd) {
    // 预先计算总大小: 帧头1 + 长度1 + cmd结构体 + 校验和2 + 帧尾1
    const size_t total_size = 1 + 1 + sizeof(MCU_cmd) + 2 + 1;
    std::vector<uint8_t> packet;
    packet.reserve(total_size);  // 预分配内存
    
    // 添加帧头(0xAA)
    packet.push_back(PROTOCOL_HEADER);
    
    // 计算帧长度
    uint8_t frame_len = sizeof(MCU_cmd) + 5;
    
    // 添加帧长度
    packet.push_back(frame_len);
    
    // 添加命令结构体(确保按字节访问，避免对齐问题)
    packet.push_back(cmd.device_byte);
    packet.push_back(cmd.cmd_type_byte);
    packet.push_back(cmd.data1);
    packet.push_back(cmd.data2);
    
    // 计算校验和(从帧头开始到数据结束)
    uint16_t checksum = calculateChecksum(packet, 0, packet.size());
    
    // 添加校验和(小端)
    packet.push_back(static_cast<uint8_t>(checksum & 0xFF));
    packet.push_back(static_cast<uint8_t>((checksum >> 8) & 0xFF));
    
    // 添加帧尾(0x55)
    packet.push_back(PROTOCOL_TAIL);
    
    return packet;
}

bool MCUProtocol::unpack(const std::vector<uint8_t>& data, MCU_cmd& cmd) {
    // TODO: 实现完整的解包逻辑
    // 这里先留空，你可以根据实际协议规范实现
    
    return false;
}

bool MCUProtocol::unpack(CircularArray<uint8_t>& unpack_array, MCU_cmd& cmd) {
    std::lock_guard<std::mutex> lock(unpack_array.mutex);
    size_t  available = unpack_array.unused_count;
    
    // 最小帧长度检查 (帧头1 + 帧长1 + 设备1 + 命令1 + 数据长度1 + 校验和2 + 帧尾1 = 8字节)
    if (available < PROTOCOL_NINE_BYTES) {
        std::cout << "go out"<< std::endl;
        return false;
    }

    // 查找帧头(0xAA)
    size_t start_pos = 0;
    uint8_t frame_len = 0;
    bool found = false;
    size_t i;
    for (i = 0; i < available; ++i) {
        if (available - i < PROTOCOL_NINE_BYTES) {  // 需要至少帧头+帧长+帧尾
            std::cout << "to less data"<< std::endl;
            break;
        }
        size_t pos = (unpack_array.read_index + i) % (unpack_array.buffer.size());        
        if (unpack_array.buffer[pos] == PROTOCOL_HEADER) {
            
            
            // 读取帧长度(不包括头尾)
            frame_len = unpack_array.buffer[(pos + 1) % (unpack_array.buffer.size())];
            
            // 检查完整帧是否可用 (frame_len已包含从帧头到帧尾的所有字节)
            if (available - i < frame_len) {
                std::cout << "bp2 "<< std::endl;
                continue;  // 剩余数据不足完整帧，或者是假帧头
            }
            
            if(unpack_array.buffer[(pos + frame_len - 1) % (unpack_array.buffer.size())] != PROTOCOL_TAIL)
            {
                std::cout << "frame_len ="<< static_cast<int>(unpack_array.buffer[(pos + frame_len) % (unpack_array.buffer.size())]) << std::endl;
                continue;  // 帧尾不匹配，继续查找下一个可能的起始位置
            }
            uint16_t calc_checksum = calculateChecksum(unpack_array.buffer, pos + 2, frame_len - 5);
            uint8_t checksum_low = unpack_array.buffer[(pos + frame_len - 2) % unpack_array.buffer.size()];
            uint8_t checksum_high = unpack_array.buffer[(pos + frame_len - 3) % unpack_array.buffer.size()];
            uint16_t received_checksum = (checksum_high << 8) | checksum_low;
            if(calc_checksum != received_checksum) {
                std::cout << "calc_checksum ="<< static_cast<int>(calc_checksum) << std::endl;
                std::cout << "received_checksum ="<< static_cast<int>(received_checksum) << std::endl;
                std::cout << "checksum error" << std::endl;
                continue;  // 校验和失败，继续查找
            }
            std::cout << "found header! "<< std::endl;
            start_pos = pos;
            found = true;
            break;
        }
    }
    if (!found) {
        //已读取数据 i+1  不足以找到一个完整的帧，丢弃这部分数据
        unpack_array.read_index = (unpack_array.read_index + i + 1) % (unpack_array.buffer.size());
        unpack_array.unused_count -= i;
        std::cout << "not found frame ,unused_count=" << unpack_array.unused_count << "  i = " << i << std::endl;
        return false;
    }
    
    // 逐个字节解析结构体字段，避免对齐问题
    cmd.device_byte = unpack_array.buffer[(start_pos + 2) % (unpack_array.buffer.size())];
    cmd.cmd_type_byte = unpack_array.buffer[(start_pos + 3) % (unpack_array.buffer.size())];
    cmd.data1 = unpack_array.buffer[(start_pos + 4) % (unpack_array.buffer.size())];
    cmd.data2 = unpack_array.buffer[(start_pos + 5) % (unpack_array.buffer.size())];
    // std::cout << "cmd.data1  = " << static_cast<int>(cmd.data1)  << std::endl;
    // std::cout << "start_pos = " << static_cast<int>(start_pos)<< std::endl;
    // for(auto it: unpack_array.buffer)
    // {
    //     std::cout << " "<< (int)it;
    // }
    // 更新读指针和未使用计数
    size_t consumed = frame_len; // frame_len已包含整个帧的长度
    unpack_array.read_index = (unpack_array.read_index + consumed) % unpack_array.buffer.size();
    unpack_array.unused_count -= consumed;
    return true;
}

uint16_t MCUProtocol::calculateChecksum(const std::vector<uint8_t>& data, size_t start, size_t len) {
    uint16_t sum = 0;
    for (size_t i = 0; i < len; ++i) {
        size_t pos = (start + i) % data.size();
        sum += data[pos];
    }
    return sum;
}

size_t MCUProtocol::findFrameStart(const std::vector<uint8_t>& data, size_t start_pos) {
    if (data.size() - start_pos < FRAME_HEADER_LEN) {
        return std::string::npos;
    }
    
    for (size_t i = start_pos; i <= data.size() - FRAME_HEADER_LEN; ++i) {
        if (data[i] == FRAME_HEADER[0] && data[i+1] == FRAME_HEADER[1]) {
            return i;
        }
    }
    
    return std::string::npos;
}