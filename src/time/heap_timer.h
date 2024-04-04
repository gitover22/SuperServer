#ifndef HEAP_TIME_H
#define HEAP_TIME_H

#include <chrono>
#include <bits/std_function.h>



typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_closck Clock;
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

};

#endif