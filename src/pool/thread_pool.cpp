#include "thread_pool.h"
ThreadPool::ThreadPool(size_t threadCount = 8):pool_(std::make_shared<Pool>()){
    assert(threadCount > 0 );
    for(size_t i=0;i<threadCount;i++){
        std::thread([pool = pool_]{
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
        }).detach();
    }
}