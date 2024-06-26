/**
 * @author  huafeng
 * @date    2024/3/28
*/
#ifndef SERVER_H
#define SERVER_H

#include <unordered_map>
#include <fcntl.h>       // fcntl()
#include <unistd.h>      // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../http/http_conn.h"
#include "../pool/thread_pool.h"
#include "../time/heap_timer.h"
#include "../pool/sqlconnpool.h"
#include "../log/log.h"
#include "epoller.h"
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
    /**
     * @brief 本函数用于创建并初始化服务器的Socket，设置相应的选项，并将其注册到epoll中等待监听。
     * @return bool 返回true表示成功初始化，返回false表示初始化过程中遇到错误。
    */
    bool Init_Socket();

    /**
     * @brief 初始化服务器的事件模式
     * @param trigger_mode [in] 触发模式，0代表默认模式，1代表边缘触发，2代表水平触发，3代表对监听和连接都使用边缘触发
    */
    void Init_EventMode(int trigger_mode);
    /**
     * @brief 添加连接客户
     * @param [in] fd       客户端的文件描述符
     * @param [in] addr     客户端地址信息
    */
    void Add_Client(int fd, sockaddr_in addr);
    /**
     * @brief 监听listenFd上的客户链接请求
     * @return void
    */
    void Deal_Listen();

    void Deal_Write(HttpConn* client);

    void Deal_Read(HttpConn* client);


    /**
     * @brief 向 fd 发送错误信息并关闭连接
     * 
     * @param fd  [in] 文件描述符，用于标识客户端
     * @param info [in] 错误信息的字符串指针
     * @return 函数不返回任何值。
    */
    void Send_Error(int fd, const char*info);

    void Extent_Time(HttpConn* client);

    /**
     * @brief 关闭连接
     * 
     * @param client [in] 指向HttpConn类的指针，表示要关闭的连接
     * @return 函数不返回任何值
     */
    void Close_Conn(HttpConn* client);

    void On_Read(HttpConn* client);
    void On_Write(HttpConn* client);
    void On_Process(HttpConn* client);
    /**
     * @brief 设置文件描述符为非阻塞模式
     * @param fd [in] 要设置的文件描述符
    */
    static int Set_fd_Nonblock(int fd);

    static const int MAX_FD = 65536;

    int server_port;
    bool openLinger;
    int timeout;  // 记录超时时间   单位：毫秒
    bool isClose;
    int listenFd;
    static char* srcDir; //web 目录的路径
    
    uint32_t listenEvent;
    uint32_t connEvent;
   
    std::unique_ptr<HeapTimer> timer;
    std::unique_ptr<ThreadPool> thread_pool;
    std::unique_ptr<Epoller> epoller; // epoller是指向Epoller类的指针
    std::unordered_map<int, HttpConn> users; // 存储连接的客户
};



#endif