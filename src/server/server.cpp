/*
@Author: huafeng
*/

#include "server.h"

Server::Server(int port_num,int trigger_mode,int timeout,
           bool quit_mode,int mysql_port,const char* mysql_user_name,
           const char* mysql_pwd,const char* db_name,int connect_pool_num,
           int thread_pool_num,bool open_log,int log_level,int log_queue_size):
           port_(port_num),openLinger(quit_mode),_timeout(timeout),isClose(false),
           timer(new HeapTimer()),threadpool(new ThreadPool(thread_pool_num)),epoller(new Epoller())
{
    srcDir = getcwd(nullptr,256);
    assert(srcDir);
    strncat(srcDir,"/web/",16); // 拼接
    HttpConn::userCount = 0;
    // http前端的路径
    HttpConn::srcDir =this->srcDir;
    // 数据库连接池
    SqlConnPool::Instance()->init("localhost",mysql_port,mysql_user_name,mysql_pwd,db_name,connect_pool_num);
    // 触发模式
    Init_EventMode(trigger_mode);
    if(!Init_Socket()) isClose = true;

    // 初试化日志
    if(open_log){
        Log::Instance()->init(log_level,"./log",".log",log_queue_size);
        if(isClose) {
            LOG_ERROR("=====init log error");
        }else{
            LOG_INFO("=========init success=======");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, quit_mode? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent & EPOLLET ? "ET": "LT"),
                            (connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connect_pool_num, thread_pool_num);

        }
    }

}