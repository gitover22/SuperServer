#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include<mutex>
#include<queue>
#include<functional>
#include<thread>
#include<assert.h>
#include<condition_variable>
class ThreadPool{
public:
    explicit ThreadPool(size_t threadCount =8);
    ThreadPool() = default;
    ThreadPool(ThreadPool&&) = default;
    ~ThreadPool();
    template<typename T>
    void AddTask(T&& task);
private:
    struct Pool{
        std::mutex mtx;
        std::condition_variable cond;
        bool isClosed;
        std::queue<std::function<void()>> tasks;
    };
    std::shared_ptr<struct Pool> pool_;
    
};

#endif