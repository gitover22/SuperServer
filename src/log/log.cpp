#include "log.h"


Log::Log(){
    lineCount =0;
    isAsync = false;
    writeThread =nullptr;
    deque =nullptr;
    toDay =0;
    fp =nullptr;
}
Log::~Log(){
    if(writeThread && writeThread->joinable()){
        while(!deque->empty()){
            deque->flush();
        }

        deque->Close();
        writeThread->join();
    }
    if(fp){
        std::lock_guard<std::mutex> locker(mtx);
        flush();
        fclose(fp);
    }
}

int Log::GetLevel(){
    std::lock_guard<std::mutex> locker(mtx);
    // this->level = 
    return level;
}
void Log::SetLevel(int level){
    std::lock_guard<std::mutex> locker(mtx);
    this->level = level;
}
void Log::init(int level,const char* path="./log",const char* suffix=".log",int maxQueueCapacity = 1024){
    isOpen = true;
    this->level =level;
    if(maxQueueCapacity >0){
        isAsync = true;
        if(!deque){
            std::unique_ptr<BlockDeque<std::string>> newDeque(new BlockDeque<std::string>());
            deque =std::move(newDeque);
            std::unique_ptr<std::thread> newThread(new std::thread(FlushLogThread));
            writeThread = std::move(newThread);
        }
    }else{
        isAsync = false;
    }
    lineCount =0 ;
    time_t timer =time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t=*sysTime;
    this->path = path;
    this->suffix = suffix;
    char fileName[LOG_NAME_LEN]={0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s", 
            path, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix);
    toDay = t.tm_mday;
    {
        std::lock_guard<std::mutex> locker(mtx);
        buff.RetrieveAll();
        if(fp){
            flush();
            fclose(fp);
        }
        fp = fopen(fileName,"a");
        if(fp == nullptr){
            mkdir(path,0777);
            fp = fopen(fileName,"a");
        }
        assert(fp!= nullptr);
    }
}