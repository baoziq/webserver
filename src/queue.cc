#include "queue.h"

template<typename T>
Queue<T>::Queue(const size_t capacity) : capacity_(capacity) {

}

template<typename T>
bool Queue<T>::Full() {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size() >= capacity_;
}

template<typename T>
bool Queue<T>::Empty(){
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template<typename T>
void Queue<T>::Push(T item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.size() >= capacity_) {
        producer_.wait(lock);
    }
    queue_.push(item);
    consumer_.notify_one();
}

template<typename T>
bool Queue<T>::Pop(T& item) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
        consumer_.wait(lock);
    }
    item = std::move(queue_.front());
    queue_.pop();
    producer_.notify_one();
    return true;
}



