#include "sqlconnpool.h"



SqlConnPool *SqlConnPool::Instance(){

}
MYSQL *SqlConnPool::GetConn(){

}
void SqlConnPool::FreeConn(MYSQL * conn){

}
int SqlConnPool::GetFreeConnCount(){

}
void SqlConnPool::init(const char* host,int port,
            const char* user,const char* pwd,
            const char* dbName,int connSize){

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