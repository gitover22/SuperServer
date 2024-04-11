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
// 默认参数只需要在函数声明时添加
void Log::init(int level,const char* path,const char* suffix,int maxQueueCapacity){
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

void Log::write(int level,const char* format,...){
    struct timeval now ={0,0};
    gettimeofday(&now,nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;
    if(toDay!=t.tm_mday || (lineCount && (lineCount%MAX_LINES ==0)))
    {
        std::unique_lock<std::mutex> locker(mtx); //unique_lock提供更多功能，比如可以手动lock和unlock
        locker.unlock();
        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail,36,"%04d_%02d_%02d",t.tm_year+1900,t.tm_mon+1,t.tm_mday);
        if(toDay != t.tm_mday){
            snprintf(newFile,LOG_NAME_LEN-72,"%s%s%s",path,tail,suffix);
            toDay = t.tm_mday;
            lineCount = 0;
        }else{
            snprintf(newFile,LOG_NAME_LEN-72,"%s/%s-%d%s",path,tail,(lineCount/MAX_LINES),suffix);
        }
        locker.lock();
        flush();
        fclose(fp);
        fp = fopen(newFile,"a");
        assert(fp != nullptr);
    }
    {
        std::unique_lock<std::mutex> locker(mtx);
        lineCount ++;
        int n = snprintf(buff.BeginWrite(),128,"%d-%02d-%02d %02d:%02d:%02d.%06ld ",t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                    t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);
        buff.HasWritten(n);
        AppendLogLevelTitle(level);
        va_start(vaList,format);
        int m = vsnprintf(buff.BeginWrite(),buff.WritableBytes(),format,vaList);
        va_end(vaList);
        buff.HasWritten(m);
        buff.Append("\n\0",2);
        if(isAsync && deque && !deque->full()){
            deque->push_back(buff.RetrieveAllToStr());
        }else{
            fputs(buff.Peek(),fp);
        }
        buff.RetrieveAll();
    }
}
void Log::AppendLogLevelTitle(int level){
    switch(level){
        case 0:
            buff.Append("[debug]: ",9);
            break;
        case 1:
            buff.Append("[info] : ", 9);
            break;
        case 2:
            buff.Append("[warn] : ", 9);
            break;
        case 3:
            buff.Append("[error]: ", 9);
            break;
        default:
            buff.Append("[info] : ", 9);
            break;
    }
}
void Log::flush(){
    if(isAsync){
        deque->flush();
    }
    fflush(fp);
}
void Log::AsyncWrite(){
    std::string str ="";
    while(deque->pop(str)){
        std::lock_guard<std::mutex> locker(mtx);
        fputs(str.c_str(),fp);

    }

}
Log* Log::Instance(){
    static Log inst;
    return &inst;
}

void Log::FlushLogThread(){
    Log::Instance()->AsyncWrite();
}

bool Log::IsOpen(){
    return isOpen;
}