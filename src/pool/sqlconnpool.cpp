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
        sql = connQue_.front();
        connQue_.pop();
    }
    return sql;
}
void SqlConnPool::FreeConn(MYSQL * conn){

}
int SqlConnPool::GetFreeConnCount(){

}
void SqlConnPool::init(const char* host,int port,
            const char* user,const char* pwd,
            const char* dbName,int connSize){
    assert(connSize>0);
    // 创建多个连接实例，并push到连接池中(queue模拟)
    for(int i=0;i<connSize;i++){
        MYSQL *sql =nullptr;
        sql = mysql_init(sql);
        if(!sql){
            LOG_ERROR("MySql init error!");
            assert(sql);
        }
        // 建立连接
        sql = mysql_real_connect(sql,host,user,pwd,dbName,port,nullptr,0);
        if(!sql){
            LOG_ERROR("mysql connect error!");
        }
        connQue_.push(sql);
    }
    MAX_CONN_=connSize;
    sem_init(&semId_,0,MAX_CONN_);
}
void SqlConnPool::ClosePool(){

}

SqlConnPool::SqlConnPool(){
    useCount_ =0 ;
    freeCount_ =0;
}
SqlConnPool::~SqlConnPool(){
    ClosePool();
}