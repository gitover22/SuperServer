#include "heap_timer.h"


HeapTimer::HeapTimer(){
    vec_heap.reserve(64); // 分配空间
}
HeapTimer::~HeapTimer(){
    clear();
}
void HeapTimer::adjust(int id, int timeout){
    assert(!vec_heap.empty() && id2idx.count(id) > 0);
    vec_heap[id2idx[id]].expires = Clock::now() +MS(timeout);
    _siftdown(id2idx[id],vec_heap.size());
}
void HeapTimer::add(int id,int timeout,const TimeoutCallBack& cb){
    assert(id >= 0);
    size_t i;
    if(id2idx.count(id) == 0){
        // 新节点
        i =vec_heap.size();
        id2idx[id] = i;
        vec_heap.push_back({id,Clock::now()+MS(timeout),cb});
        _siftup(i);
    }else{
        // 已有 修改堆
        i =id2idx[id];
        vec_heap[i].expires =Clock::now() +MS(timeout);
        vec_heap[i].cb =cb;
        if(!_siftdown(i,vec_heap.size())){
            _siftup(i);
        }
    }
}
void HeapTimer::doWork(int id){
    if(vec_heap.empty() || id2idx.count(id) == 0) return;
    size_t i =id2idx[id];
    TimerNode node = vec_heap[i];
    node.cb();
    _delete(i);
}
void HeapTimer::clear(){
    id2idx.clear();
    vec_heap.clear();
}
void HeapTimer::tick(){
    if(vec_heap.empty()) return;
    while(!vec_heap.empty()){
        TimerNode node =vec_heap.front();
        if(std::chrono::duration_cast<MS>(node.expires - Clock::now()).count()>0){
            break;
        }
        node.cb();
        pop();
    }

}
void HeapTimer::pop(){
    assert(!vec_heap.empty());
    _delete(0);
}
int HeapTimer::GetNextTick(){
    tick();
    size_t res = -1;
    if(!vec_heap.empty()){
        res = std::chrono::duration_cast<MS>(vec_heap.front().expires-Clock::now()).count();
        if(res < 0 ) res = 0;
    }
    return res;

}

void HeapTimer::_delete(size_t i){
    assert(!vec_heap.empty() && i>=0 &&i<vec_heap.size());
    size_t index = i;
    size_t n =vec_heap.size() - 1;
    assert(index<=n);
    if(index<n){
        _swap(index,n);
        if(!_siftdown(index,n)){
            _siftup(index);
        }
    }
    id2idx.erase(vec_heap.back().id);
    vec_heap.pop_back();
}
void HeapTimer::_siftup(size_t i){
    assert(i>=0 && i<vec_heap.size());
    size_t j = (i-1) / 2;
    while(j>=0){
        if(vec_heap[j] < vec_heap[i]) break;
        _swap(i,j);
        i = j;
        j= (i-1)/2;
    }
}
bool HeapTimer::_siftdown(size_t index,size_t n){
    assert(index >=0 && index<vec_heap.size());
    assert(n>=0&&n<=vec_heap.size());
    size_t i = index;
    size_t j = i*2 + 1;
    while(j<n){
        if(j+1 <n&&vec_heap[j+1] <vec_heap[j]) j++;
        if(vec_heap[i] <vec_heap[j]) break;
        _swap(i,j);
        i = j;
        j = i*2 +1;
    }
    return i > index;
}
void HeapTimer::_swap(size_t i,size_t j){
    assert(i>=0 && i<vec_heap.size());
    assert(j>=0 && j<vec_heap.size());
    std::swap(vec_heap[i],vec_heap[j]);
    id2idx[vec_heap[i].id] =i;
    id2idx[vec_heap[j].id] =j;

}