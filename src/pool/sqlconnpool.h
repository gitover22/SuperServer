#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <queue>
#include <mutex>
#include <mysql/mysql.h>
#include <semaphore.h>
#include "../log/log.h"
class SqlConnPool{
public:
    /**
     * @brief 单例模式
     * @return SqlConnPool的实例地址
    */
    static SqlConnPool *Instance();
    /**
     * @brief 从数据库连接池中获得一个连接实例
     * @return 返回实例(MYSQL *)地址 
    */
    MYSQL *GetConn();
    void FreeConn(MYSQL * sql);
    int GetFreeConnCount();
    void init(const char* host,int port,
              const char* user,const char* pwd,
              const char* dbName,int connSize);
    void ClosePool();
private:
    SqlConnPool();
    ~SqlConnPool();
    int MAX_CONN_; // 最大连接数
    int useCount_; // 已使用连接数
    int freeCount_; //剩余可用连接数
    std::queue<MYSQL*> connQue_; //连接池
    std::mutex mtx_; // 互斥量
    sem_t semId_; //信号量
};

#endif