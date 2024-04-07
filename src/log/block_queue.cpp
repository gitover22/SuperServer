#include "block_queue.h"
// 类成员函数的定义
template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity):capacity(MaxCapacity){
    assert(MaxCapacity > 0); // 容量必须大于零
    isClose = false;
}

template<class T>
BlockDeque<T>::~BlockDeque(){
    Close();
}

template<class T>
void BlockDeque<T>::Close(){
    {
        std::lock_guard<std::mutex> locker(mtx);
        deq.clear();
        isClose =true;

    }
    condProducer.notify_all();
    condConsumer.notify_all();
}

template<class T>
void BlockDeque<T>::flush(){
    condConsumer.notify_one();
}

template<class T>
void BlockDeque<T>::clear(){
    std::lock_guard<std::mutex> locker(mtx);
    deq.clear();

}

template<class T>
T BlockDeque<T>::front(){
    std::lock_guard<std::mutex> locker(mtx);
    return deq.front();
}

template<class T>
T BlockDeque<T>::back(){
    std::lock_guard<std::mutex> locker(mtx);
    return deq.back();
}

template<class T>
size_t BlockDeque<T>::size(){
    std::lock_guard<std::mutex> locker(mtx);
    return deq.size();
}

template<class T>
size_t BlockDeque<T>::get_capacity(){
    std::lock_guard<std::mutex> locker(mtx);
    return this->capacity;
}

template<class T>
void BlockDeque<T>::push_back(const T&item){
    std::unique_lock<std::mutex> locker(mtx);
    while(deq.size() >= capacity){
        condProducer.wait(locker);
    }
    deq.push_back(item);
    condProducer.notify_one();
}
