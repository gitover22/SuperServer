#ifndef LOG_H
#define LOG_H
#include "./block_queue.h"
#include "../buffer/buffer.h"
#include <bits/types/FILE.h>
#include <memory>
#include <thread>
#include <mutex>
#include <sys/stat.h>
#include <assert.h>
#include <sys/time.h>
#include <stdarg.h>
class Log{
public:
    void init(int level,const char* path="./log",const char* suffix=".log",int maxQueueCapacity = 1024);
    static Log* Instance();

    static void FlushLogThread();

    void write(int level,const char* format,...);
    void flush();
    int GetLevel();
    void SetLevel(int level);
    bool IsOpen();
private:
    Log(); // 单例模式
    void AppendLogLevelTitle(int level);
    virtual ~Log();
    void AsyncWrite();


    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path;
    const char* suffix;
    int max_lines;
    int lineCount;
    int toDay;
    bool isOpen;
    Buffer buff;
    int level;
    bool isAsync; // 是否异步
    FILE* fp;
    std::unique_ptr<BlockDeque<std::string>> deque;
    std::unique_ptr<std::thread> writeThread;
    std::mutex mtx;


};


#define LOG_BASE(level,format, ...) \
    do {\
        Log* log = Log::Instance();\
        if(log->IsOpen() && (log->GetLevel() <= level)){ \
            log->write(level,format,##_VA_ARGS__); \
            log->flush();\
        }\
    }while(0);

#define LOG_DEBUG(format, ...) do {LOG_BASE(0,format,##_VA_ARGS__)}while(0);
#define LOG_INFO(format, ...) do {LOG_BASE(1,format,##_VA_ARGS__)}while(0);
#define LOG_WARN(format, ...) do {LOG_BASE(2,format,##_VA_ARGS__)}while(0);
#define LOG_ERROR(format, ...) do {LOG_BASE(3,format,##_VA_ARGS__)}while(0);


#endif