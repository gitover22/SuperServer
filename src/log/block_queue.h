#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <condition_variable>
#include <bits/std_mutex.h>
#include <deque>
// 类模板 自定义的一个双端数组
template<class T>
class BlockDeque{
public:
//类成员函数的声明
    explicit BlockDeque(size_t MaxCapacity  = 1000);
    ~BlockDeque();
    void clear();
    bool empty();
    bool full();
    void Close();
    size_t size();
    size_t get_capacity();
    T front();
    T back();
    void push_back(const T& item);
    void push_front(const T& item);
    bool pop(T& item);
    bool pop(T& item,int timeout);
    void flush();
private:
    std::deque<T> deq;
    size_t capacity;
    std::mutex mtx;
    bool isClose;
    std::condition_variable condConsumer;
    std::condition_variable condProducer;

};








#endif