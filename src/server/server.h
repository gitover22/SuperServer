/**
 * @author  huafeng
 * @date    2024/3/28
*/
#ifndef SERVER_H
#define SERVER_H

#include <netinet/in.h>
#include <memory>
#include <bits/unordered_map.h>
#include "../http/http_conn.h"
#include "../pool/thread_pool.h"
#include "epoller.h"
#include "../time/heap_timer.h"
#include "../pool/sqlconnpool.h"
#include "../log/log.h"
class Server{
public:
    /*!
    * @brief Server类的有参构造函数，用于设置服务器各种参数
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

    /*!
    * @brief Server的析构函数，释放资源
    *
    */
    ~Server();

    /*!
    * @brief 启动服务器的入口函数
    *
    */
    void Start();
private:
    /*!
    * @brief socket的初试化
    *
    */
    bool Init_Socket();

    /*!
    * @brief 初试化事件触发模式
    * @param trigger_mode [in] 接受的触发模式
    */
    void Init_EventMode(int trigger_mode);
    /**
     * @brief 添加连接客户
     * @param [in] fd       文件描述符
     * @param [in] addr     客户端地址信息
    */
    void Add_Client(int fd, sockaddr_in addr);

    void Deal_Listen();

    void Deal_Write(HttpConn* client);

    void Deal_Read(HttpConn* client);


    /**
     * @brief 发送错误信息
     * @param [in] fd       文件描述符
     * @param [in] info     要发送的信息
    */
    void Send_Error(int fd, const char*info);

    void Extent_Time(HttpConn* client);

    /**
     * @brief 关闭客户连接
     * @param [in] client     客户端连接信息
    */
    void Close_Conn(HttpConn* client);

    void On_Read(HttpConn* client);
    void On_Write(HttpConn* client);
    void On_Process(HttpConn* client);

    static int Set_fd_Nonblock(int fd);

    static const int MAX_FD = 65536;

    int port_;
    bool openLinger;
    int _timeout;  // 毫秒
    bool isClose;
    int listenFd;
    char* srcDir;
    
    uint32_t listenEvent;
    uint32_t connEvent;
   
    std::unique_ptr<HeapTimer> timer;
    std::unique_ptr<ThreadPool> threadpool;
    std::unique_ptr<Epoller> epoller;
    std::unordered_map<int, HttpConn> users; // 存储连接的客户
};



#endif