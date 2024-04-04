#ifndef HEAP_TIME_H
#define HEAP_TIME_H

#include <chrono>
#include <functional>



typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;

struct TimerNode{
    int id;
    TimeStamp expires;
    TimeoutCallBack cb;
    bool operator<(const TimerNode& t){
        return expires < t.expires;
    }
};


class HeapTimer{
public:
    HeapTimer();
    ~HeapTimer();
    void adjust(int id, int newExpires);
    void add(int id,int timeOut,const TimeoutCallBack& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();
private:
    void del_(size_t i);
    void siftup_(size_t index);
    bool siftdown_(size_t index,size_t n);
    void SwapNode_(size_t i,size_t j);
    std::vector<TimerNode> heap_;
    std::unordered_map<int,size_t> ref_; // unordered不能插入重复的key
};

#endif