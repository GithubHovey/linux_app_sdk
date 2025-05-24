#ifndef MCU_PROTOCOL_H
#define MCU_PROTOCOL_H

#include <vector>
#include <cstdint>
#include "ringbuffer.h"
#define PROTOCOL_NINE_BYTES 9
#define PROTOCOL_HEADER 0xAA  // 帧头标识
#define PROTOCOL_TAIL 0x55    // 帧尾标识
//帧头 1字节 -- 帧长 1字节 -- 设备字段（1字节）--命令类型(1字节)--数据字段（2字节）--校验和(2字节)--帧尾 1字节
// 假设的MCU命令结构体
typedef struct {
    // --- Device字段（1字节，位域拆分）---
    union {
        uint8_t device_byte;  // 整体访问
        struct {
            uint8_t device_type : 4;  // 低4位：设备类型 (0~15)
            uint8_t device_id   : 4;   // 高4位：设备序号 (0~15)
        };
    };

    // --- Command类型（1字节，位域拆分）---
    union {
        uint8_t cmd_type_byte;  // 整体访问
        struct {
            uint8_t is_data     : 1;  // 最低位：0=命令, 1=数据
            uint8_t type        : 7;  // 高7位：数据类型/命令子类型 (0~127)
        };
    };

    uint8_t   data1;  // 数据长度（0~255）
    uint8_t   data2;       
} MCU_cmd;

static_assert(sizeof(MCU_cmd) == 4, "MCU_cmd structure size mismatch, check packing/alignment");

struct MCU_frame {
    uint8_t header;  // 帧头标识（0xAA）
    uint8_t   len;     // 帧长度（不包括头尾标识）
    MCU_cmd cmd;     // MCU命令结构体
    uint8_t  checksum; // 校验和（16位，低字节在前）
    uint16_t tail;    // 帧尾标识（0x55）
};

class MCUProtocol {
public:
    // 数据打包
    std::vector<uint8_t> pack(const MCU_cmd& cmd);
    
    // 数据解包
    bool unpack(const std::vector<uint8_t>& data, MCU_cmd& cmd);
    bool unpack(CircularArray<uint8_t>& unpack_array, MCU_cmd& cmd);
    
    // 计算校验和
    uint16_t calculateChecksum(const std::vector<uint8_t>& data, size_t start, size_t len);
    
    // 查找帧头
    static size_t findFrameStart(const std::vector<uint8_t>& data, size_t start_pos = 0);
};

#endif 