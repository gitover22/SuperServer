#include "sqlconnpool.h"


SqlConnPool *SqlConnPool::Instance(){
    // 单例模式
    static SqlConnPool connPool;
    return &connPool;
}
MYSQL *SqlConnPool::GetConn(){
    MYSQL *sql =nullptr;
    if(connQue_.empty()){
        // 连接池全被使用
        LOG_WARN("sqlConnPool no margin!");
        return nullptr;
    }
    sem_wait(&semId_);
    {
        std::lock_guard<std::mutex> locker(mtx_);
        sql = connQue_.front(); // 取出一个连接
        connQue_.pop();
    }
    return sql;
}
void SqlConnPool::FreeConn(MYSQL * sql){
    assert(sql);
    std::lock_guard<std::mutex>locker(mtx_);
    connQue_.push(sql); // 释放连接: push到队列中（假释放）
    sem_post(&semId_); //唤醒+1
}
int SqlConnPool::GetFreeConnCount(){
    std::lock_guard<std::mutex> locker(mtx_);
    return connQue_.size();
}
void SqlConnPool::init(const char* host,int port,
            const char* user,const char* pwd,
            const char* dbName,int connSize){
    assert(connSize>0);
    // 创建多个连接实例，并push到连接池中
    for(int i=0;i<connSize;i++){
        MYSQL *sql =nullptr;
        sql = mysql_init(sql);  // MYSQL提供的库函数
        if(!sql){
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        // 建立连接
        sql = mysql_real_connect(sql,host,user,pwd,dbName,port,nullptr,0); // MYSQL提供的库函数
        if(!sql){
            LOG_ERROR("mysql connect error!");
        }
        connQue_.push(sql);
    }
    MAX_CONN_=connSize;
    sem_init(&semId_,0,MAX_CONN_);
}
void SqlConnPool::ClosePool(){
    std::lock_guard<std::mutex> locker(mtx_);
    // 轮番从池中取出并关闭
    while(!connQue_.empty()){
        auto item =connQue_.front();
        connQue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

SqlConnPool::SqlConnPool(){
    useCount_ =0 ;
    freeCount_ =0;
}
SqlConnPool::~SqlConnPool(){
    ClosePool();
}