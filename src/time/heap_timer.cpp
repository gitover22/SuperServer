#include "heap_timer.h"


HeapTimer::HeapTimer(){
    heap_.reserve(64); // 分配空间
}
HeapTimer::~HeapTimer(){
    clear();
}
void HeapTimer::adjust(int id, int newExpires){

}
void HeapTimer::add(int id,int timeOut,const TimeoutCallBack& cb){

}
void HeapTimer::doWork(int id){

}
void HeapTimer::clear(){
    ref_.clear();
    heap_.clear();
}
void HeapTimer::tick(){

}
void HeapTimer::pop(){

}
int HeapTimer::GetNextTick(){

}

void HeapTimer::del_(size_t i){

}
void HeapTimer::siftup_(size_t i){
    assert(i>=0 && i<heap_.size());
    size_t j = (i-1) / 2;
    while(j>=0){
        if(heap_[j] < heap_[i]) break;
        SwapNode_(i,j);
        i = j;
        j= (i-1)/2;
    }
}
bool HeapTimer::siftdown_(size_t index,size_t n){
    assert(index >=0 && index<heap_.size());
    assert(n>=0&&n<=heap_.size());
    size_t i = index;
    size_t j = i*2 + 1;
    while(j<n){
        if(j+1 <n&&heap_[j+1] <heap_[j]) j++;
        if(heap_[i] <heap_[j]) break;
        SwapNode_(i,j);
        i = j;
        j = i*2 +1;
    }
    return i > index;
}
void HeapTimer::SwapNode_(size_t i,size_t j){
    assert(i>=0 && i<heap_.size());
    assert(j>=0 && j<heap_.size());
    std::swap(heap_[i],heap_[j]);
    ref_[heap_[i].id] =i;
    ref_[heap_[j].id] =j;

}