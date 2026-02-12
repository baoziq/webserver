//
// Created by qsj on 2026/2/9.
//

#ifndef WEBSERVER_QUEUE_H
#define WEBSERVER_QUEUE_H

#include <condition_variable>
#include <queue>
#include <string>
#include <mutex>

template <typename T>
class Queue {
public:
    explicit Queue(size_t capacity);
    ~Queue() = default;
    bool Pop(T& item);
    void Push(T item);
    bool Empty();
    bool Full();
private:
    size_t capacity_;
    std::condition_variable consumer_;
    std::condition_variable producer_;
    std::mutex mutex_;
    std::queue<T> queue_;

};
#endif //WEBSERVER_QUEUE_H