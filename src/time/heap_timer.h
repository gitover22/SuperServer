#ifndef HEAP_TIME_H
#define HEAP_TIME_H

#include <chrono>
#include <functional>
#include <assert.h>
#include <vector>
#include <unordered_map>


typedef std::function<void()> TimeoutCallBack; //回调函数的类型定义，用于执行定时器到时的操作
typedef std::chrono::high_resolution_clock Clock; // 高分别率时钟
typedef std::chrono::milliseconds MS; // 时间单位： 毫秒
typedef Clock::time_point TimeStamp; // 时间点类型

struct TimerNode{
    int id; // 定时器标识
    TimeStamp expires; // 定时器到期的时间点
    TimeoutCallBack cb; // 该定时器到期时执行的回调函数
    // 重载 < 用于比较两个定时器的到期时间
    bool operator<(const TimerNode& t){
        return expires < t.expires;
    }
};


class HeapTimer{
public:
    HeapTimer();
    ~HeapTimer();
    /**
     * @brief 调整标识为id的定时器的到期时间
    */
    void adjust(int id, int newExpires);
    /**
     * @brief 添加新的定时器
    */
    void add(int id,int timeOut,const TimeoutCallBack& cb);
    /**
     * @brief 执行指定id的定时器任务,并删除该定时器
    */
    void doWork(int id);
    /**
     * @brief 清空所有定时器和引用
    */
    void clear();
    /**
     * @brief 检查并执行所有已到期的定时器
    */
    void tick();
    /**
     * @brief 删除堆顶（最早到期）的定时器。
    */
    void pop();
    /**
     * @brief 计算到下一个定时器到期所需的时间
    */
    int GetNextTick();
private:
    /**
     * @brief 删除指定位置的定时器
    */
    void del_(size_t i);
    /**
     * @brief 向上调整堆，以维护最小堆性质
    */
    void siftup_(size_t index);
    /**
     * @brief 向下调整堆，以维护最小堆性质
    */
    bool siftdown_(size_t index,size_t n);
    /**
     * @brief 交换两个节点的位置，并更新它们在引用映射中的位置
    */
    void SwapNode_(size_t i,size_t j);
    std::vector<TimerNode> heap_; // 存储定时器节点的向量，组织成最小堆
    std::unordered_map<int,size_t> ref_; // 映射表，用于根据定时器id快速找到其在堆中的位置
};

#endif