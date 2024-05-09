#include "thread_pool.h"
ThreadPool::ThreadPool(size_t threadCount):pool_(std::make_shared<Pool>()){
    assert(threadCount > 0 );
    for(size_t i=0;i<threadCount;i++){
        std::thread([pool = this->pool_]{
            std::unique_lock<std::mutex> locker(pool->mtx);
            while(true){
                if(!pool->tasks.empty()){
                    auto task = std::move(pool->tasks.front()); // task是一个可执行函数
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                }
                else if(pool->isClosed) break;
                else pool->cond.wait(locker);
             }
        }).detach(); // 设置该线程为游离态，系统自动回收游离态线程，无需其他线程使用join回收
    }
    
}

ThreadPool::~ThreadPool(){
    if(static_cast<bool>(pool_)){
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->isClosed = true;
        }
        pool_->cond.notify_all();
    }   
}

