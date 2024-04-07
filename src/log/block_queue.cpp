#include "block_queue.h"

template<class T>
BlockDeque<T>::BlockDeque(size_t MaxCapacity):capacity(MaxCapacity){
    assert(MaxCapacity > 0); // 容量必须大于零
    isClose = false;
};

template<class T>
BlockDeque<T>::~BlockDeque(){
    Close();
};

template<class T>
void BlockDeque<T>::Close(){
    {
        std::lock_guard<std::mutex> locker(mtx);
        deq.clear();
        isClose =true;

    }
    condProducer.notify_all();
    condConsumer.notify_all();
};