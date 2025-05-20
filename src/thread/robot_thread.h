#pragma
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <atomic>
#include "ringbuffer.h"
#include "mcu_protocol.h"
void gui_thread();
void AIchat_thread();
void test_thread(RingBuffer<MCU_cmd>& rx_buf, RingBuffer<MCU_cmd>& tx_buf);
void serial_com_thread(RingBuffer<MCU_cmd>& rx_ring, RingBuffer<MCU_cmd>& tx_ring);