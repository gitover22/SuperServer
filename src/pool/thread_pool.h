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
        bool isClosed; // 线程池是否关闭
        std::queue<std::function<void()>> tasks; // 任务队列
    };
    std::shared_ptr<struct Pool> pool_;
    
};
template<typename T>
void ThreadPool::AddTask(T&& task){
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        pool_->tasks.emplace(std::forward<T>(task)); // 完美转发

    }
    pool_->cond.notify_one(); // 唤醒一个等待在条件变量 cond 上的线程
}
#endif