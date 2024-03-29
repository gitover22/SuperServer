#ifndef LOG_H
#define LOG_H

#include "../buffer/buffer.h"
#include <bits/types/FILE.h>
#include <memory>
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
private:
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
    bool isAsync;
    FILE* fp;



};







#endif