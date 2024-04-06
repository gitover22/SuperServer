#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <semaphore.h>
#include <queue>
#include <mutex>

class SqlConnPool{
public:
    static SqlConnPool *Instance();
    MYSQL *GetConn();
    void FreeConn(MYSQL * conn);
    int GetFreeConnCount();
    void init(const char* host,int port,
              const char* user,const char* pwd,
              const char* dbName,int connSize);
    void ClosePool();
private:
    SqlConnPool();
    ~SqlConnPool();
    int MAX_CONN_;
    int useCount_;
    int freeCount_;
    std::queue<MYSQL*> connQue_;
    std::mutex mtx_;
    sem_t semId_;
};





#endif