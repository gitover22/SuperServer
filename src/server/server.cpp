/*
@Author: huafeng
*/

#include "server.h"
char* Server::srcDir = "/home/huafeng/SuperServer/web/";
Server::Server(int port_num,int trigger_mode,int time,
           bool quit_mode,int mysql_port,const char* mysql_user_name,
           const char* mysql_pwd,const char* db_name,int connect_pool_num,
           int thread_pool_num,bool open_log,int log_level,int log_queue_size):
           server_port(port_num),openLinger(quit_mode),timeout(time),isClose(false),
           timer(new HeapTimer()),thread_pool(new ThreadPool(thread_pool_num)),epoller(new Epoller())
{
    assert(srcDir);
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
            LOG_INFO("Port:%d, OpenLinger: %s", server_port, quit_mode? "true":"false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                            (listenEvent & EPOLLET ? "ET": "LT"),
                            (connEvent & EPOLLET ? "ET": "LT"));
            LOG_INFO("LogSys level: %d", log_level);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connect_pool_num, thread_pool_num);

        }
    }

}
Server::~Server(){
    close(listenFd);
    isClose =true;
    free(srcDir);
    SqlConnPool::Instance()->ClosePool(); // 关闭数据库连接池
}


/**
 * @brief 初始化服务器的事件模式
 * @param trigMode 触发模式，0代表默认模式，1代表边缘触发，2代表水平触发，3代表对监听和连接都使用边缘触发
 */
void Server::Init_EventMode(int trigMode) {
    // 默认的事件模式设置
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP; // EPOLLONESHOT表示只监听一次事件
    switch (trigMode)
    {
    case 0:
        // 默认模式，不修改事件模式
        break;
    case 1:
        // 对连接事件使用边缘触发
        connEvent |= EPOLLET;
        break;
    case 2:
        // 对监听事件使用边缘触发
        listenEvent |= EPOLLET;
        break;
    case 3:
        // 对监听和连接事件都使用边缘触发
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    default:
        // 对监听和连接事件都使用边缘触发，处理非法的trigMode值
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    }
    // 标记连接是否使用边缘触发
    HttpConn::isET = (connEvent & EPOLLET);
}

void Server::Start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose) { LOG_INFO("========== Server start =========="); }
    while(!isClose) {
        if(timeout > 0) {
            timeMS = timer->GetNextTick();
        }
        // 等待事件   标准的epoll处理流程
        int eventCnt = epoller->Wait(timeMS);
        for(int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = epoller->GetEventFd(i);
            uint32_t events = epoller->GetEvents(i);
            if(fd == listenFd) {
                Deal_Listen();
            }
            else if(events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users.count(fd) > 0);
                Close_Conn(&users[fd]);
            }
            else if(events & EPOLLIN) {
                assert(users.count(fd) > 0);
                Deal_Read(&users[fd]);
            }
            else if(events & EPOLLOUT) {
                assert(users.count(fd) > 0);
                Deal_Write(&users[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}
/**
 * @brief 向 fd 发送错误信息并关闭连接
 * 
 * @param fd  [in] 文件描述符，用于标识客户端连接
 * @param info [in] 错误信息的字符串指针
 * @return 函数不返回任何值。
 */
void Server::Send_Error(int fd, const char*info) {
    assert(fd > 0); 
    
    // 尝试发送错误信息给fd
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) { // 发送失败
        LOG_WARN("send error to client[%d] error!", fd);
    }
    
    close(fd); // 关闭文件描述符，即断开连接
}

void Server::Close_Conn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller->DelFd(client->GetFd());
    client->Close();
}

void Server::Add_Client(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users[fd].init(fd, addr);
    if(timeout > 0) {
        timer->add(fd, timeout, std::bind(&Server::Close_Conn, this,&users[fd]) );
    }
    epoller->AddFd(fd, EPOLLIN | connEvent);
    Set_fd_Nonblock(fd);
    LOG_INFO("Client[%d] in!", users[fd].GetFd());
}

int Server::Set_fd_Nonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}

void Server::Deal_Listen(){
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do{
        int fd = accept(listenFd, (struct sockaddr *)&addr, &len);
        if(fd <= 0) { return;}
        else if(HttpConn::userCount >= MAX_FD) {
            Send_Error(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        Add_Client(fd, addr);
    }while(listenEvent & EPOLLET);
}

void Server::Deal_Read(HttpConn* client) {
    assert(client);
    Extent_Time(client);
    thread_pool->AddTask(std::bind(&Server::On_Read, this, client));
}
void Server::Deal_Write(HttpConn* client) {
    assert(client);
    Extent_Time(client);
    thread_pool->AddTask(std::bind(&Server::On_Write, this, client));
}

void Server::Extent_Time(HttpConn *client){
    assert(client);
    if(timeout > 0) {
        timer->adjust(client->GetFd(),timeout);
    }
}

void Server::On_Read(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if(ret <= 0 && readErrno != EAGAIN) {
        Close_Conn(client);
        return;
    }
    On_Process(client);
}

void Server::On_Process(HttpConn* client) {
    if(client->process()) {
        epoller->ModFd(client->GetFd(), connEvent | EPOLLOUT);
    } else {
        epoller->ModFd(client->GetFd(), connEvent | EPOLLIN);
    }
}

void Server::On_Write(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if(client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if(client->IsKeepAlive()) {
            On_Process(client);
            return;
        }
    }
    else if(ret < 0) {
        if(writeErrno == EAGAIN) {
            /* 继续传输 */
            epoller->ModFd(client->GetFd(), connEvent | EPOLLOUT);
            return;
        }
    }
    Close_Conn(client);
}

/**
 * @brief 本函数用于创建并初始化服务器的Socket，设置相应的选项，并将其注册到epoll中等待监听。
 * @return bool 返回true表示成功初始化，返回false表示初始化过程中遇到错误。
 */
bool Server::Init_Socket(){
    int ret;
    struct sockaddr_in addr;
    // 检查端口号是否在有效范围内
    if(server_port >65535 || server_port <1024){
        LOG_ERROR("Port:%d error!",server_port);
        return false;
    }
    // 设置Socket地址结构体
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr =htonl(INADDR_ANY);
    addr.sin_port =htons(server_port);
    
    struct linger optLinger ={0};  // 初始化linger选项，用于控制Socket关闭方式
    if(openLinger){
        // 设置为优雅关闭，确保所有数据被发送或超时
        optLinger.l_onoff = 1;
        optLinger.l_linger = 1;
    }
    
    // 创建Socket
    listenFd =socket(AF_INET,SOCK_STREAM,0);
    if(listenFd < 0){
        LOG_ERROR("CREATE socket error!",server_port);
        return false;
    }
    
    // 设置Socket选项：linger
    ret =setsockopt(listenFd,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
    if(ret < 0){
        close(listenFd);
        LOG_ERROR("init linger error!");
        return false;
    }
    
    // 设置Socket选项：端口复用 以允许新的服务器进程绑定到先前使用的端口上
    int optval = 1;
    ret = setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int));
    if(ret == -1){
        LOG_ERROR("setsocketopt error!");
        close(listenFd);
        return false;
    }
    
    // 绑定端口
    ret = bind(listenFd,(struct sockaddr *)&addr,sizeof(addr));
    if(ret < 0){
        LOG_ERROR("bind port:%d error!",server_port);
        close(listenFd);
        return false;
    }
    
    // 监听端口
    ret = listen(listenFd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", server_port);
        close(listenFd);
        return false;
    }
    
    // 将监听Socket添加到epoll中
    ret = epoller->AddFd(listenFd,  listenEvent | EPOLLIN);
    if(ret == 0) {
        LOG_ERROR("Add listen error!");
        close(listenFd);
        return false;
    }
    
    // 设置监听Socket为非阻塞模式
    Set_fd_Nonblock(listenFd);
    
    LOG_INFO("Server port:%d", server_port);
    return true;
}