#ifndef SERVER_H
#define SERVER_H

class Server{
public:
    /*!
    * @brief Server类的构造函数
    *
    * @param [in] port_num          该服务器的端口
    * @param [in] trigger_mode      触发模式 ET LT
    * @param [in] timeout           超时限时时间
    * @param [in] quit_mode         是否优雅退出 
    * @param [in] mysql_port        mysql数据库的服务端口
    * @param [in] mysql_user_name   数据库的用户名
    * @param [in] mysql_pwd         用户密码
    * @param [in] db_name           数据库名称
    * @param [in] connect_pool_num  连接池数量
    * @param [in] thread_pool_num   线程池数量
    * @param [in] open_log          是否打开日志
    * @param [in] log_level         日志等级
    * @param [in] log_queue_size    日志队列容量
    * @retval 
    */
    Server(int port_num,int trigger_mode,int timeout,
           bool quit_mode,int mysql_port,const char* mysql_user_name,
           const char* mysql_pwd,const char* db_name,int connect_pool_num,
           int thread_pool_num,bool open_log,int log_level,int log_queue_size);


};



#endif