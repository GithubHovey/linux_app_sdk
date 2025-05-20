#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>
#include <mutex>
#include <condition_variable>

template <typename T>
struct CircularArray {
    // 构造函数，初始化vector和指针位置
    explicit CircularArray(size_t size) 
        : buffer(size), read_index(0), write_index(0), unused_count(0) {}

    // 成员变量
    std::vector<T> buffer;
    size_t read_index;
    size_t write_index;
    size_t unused_count;
    std::mutex mutex;
};


template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t size) 
        : buffer_(size), capacity_(size), head_(0), tail_(0), count_(0) {}

    bool push(const T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == capacity_) {
            return false;
        }
        buffer_[tail_] = item;
        tail_ = (tail_ + 1) % capacity_;
        ++count_;
        cv_.notify_one();
        return true;
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (count_ == 0) {
            cv_.wait_for(lock, std::chrono::milliseconds(100));
            if (count_ == 0) {
                return false;
            }
        }
        item = buffer_[head_];
        head_ = (head_ + 1) % capacity_;
        --count_;
        return true;
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return count_ == 0;
    }

    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_ = count_ = 0;
    }

private:
    std::vector<T> buffer_;
    size_t capacity_;
    size_t head_;
    size_t tail_;
    size_t count_;
    mutable std::mutex mutex_;
    std::condition_variable cv_;
};

#endif // RINGBUFFER_H