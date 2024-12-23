/*
@Author: huafeng
*/

#include "server.h"
// #include <filesystem> 
char* Server::srcDir = "/home/zouguoqiang/SuperServer/web"; // TODO(huafeng):using std::filesystem::current_path().c_str() to get the current path
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



void Server::Init_EventMode(int trigMode) {
// 水平触发（Level Triggered, LT）
// 行为：在这种模式下，只要被监控的文件描述符的状态仍然满足请求的条件，epoll_wait 就会重复通知该事件。例如，如果指定监听读事件，只要缓冲区中还有数据未读，epoll_wait 就会再次通知可读事件。
// 适用场景：在水平触发模式下，应用程序可以不用立即处理所有数据，也不需要非阻塞 I/O，因为 epoll_wait 会在数据依然可读或可写时再次通知。
// 优点：编程模型简单，易于理解和实现。
// 缺点：可能会导致更多的 epoll_wait 调用返回，因为只要条件满足，事件就会被反复报告，从而性能降低
// 边缘触发（Edge Triggered, ET）
// 行为：在边缘触发模式下，只有状态变化时（例如从不可读变为可读）epoll_wait 才会通知该事件一次。这意味着在 epoll_wait 通知之后，需要一次性处理掉所有的数据，直到对方发送更多的数据或状态再次改变。
// 适用场景：适用于非阻塞 I/O，这种模式可以减少 epoll_wait 的调用次数，提高应用程序效率，特别是在高负载时。
// 优点：减少了事件通知的次数，只在状态真正发生变化时才触发，可以显著提高程序的效率和性能。
// 缺点：编程复杂度高，需要确保每次事件通知时都完全处理完相关的数据，否则可能会错过事件处理。
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
    // 从epoll中删除该文件描述符
    epoller->Delete_Fd(client->GetFd());
    client->Close();
}

void Server::Add_Client(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users[fd].init(fd, addr);
    if(timeout > 0) {
        timer->add(fd, timeout, std::bind(&Server::Close_Conn, this,&users[fd]) );
    }
    epoller->Add_Fd(fd, EPOLLIN | connEvent);
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
        epoller->Modify_Fd(client->GetFd(), connEvent | EPOLLOUT);
    } else {
        epoller->Modify_Fd(client->GetFd(), connEvent | EPOLLIN);
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
            epoller->Modify_Fd(client->GetFd(), connEvent | EPOLLOUT);
            return;
        }
    }
    Close_Conn(client);
}


bool Server::Init_Socket(){
    int ret;
    struct sockaddr_in addr;
    // 端口号必须 [1024,65535]
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
    
    // 设置socket属性：linger选项控制socket关闭方式
    ret =setsockopt(listenFd,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger));
    if(ret < 0){
        close(listenFd);
        LOG_ERROR("init linger error!");
        return false;
    }
    
    // 设置socket属性：端口复用 以允许新的服务器进程绑定到先前使用的端口上
    int optval = 1;
    ret = setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int));
    if(ret == -1){
        LOG_ERROR("setsocketopt error!");
        close(listenFd);
        return false;
    }
    
    // 绑定socket文件描述符和具体的sockaddr结构体
    ret = bind(listenFd,(struct sockaddr *)&addr,sizeof(addr));
    if(ret < 0){
        LOG_ERROR("bind port:%d error!",server_port);
        close(listenFd);
        return false;
    }
    
    // 监听端口，创建监听队列
    ret = listen(listenFd, 6);
    if(ret < 0) {
        LOG_ERROR("Listen port:%d error!", server_port);
        close(listenFd);
        return false;
    }
    
    // 将监听Socket添加到epoll中
    ret = epoller->Add_Fd(listenFd,  listenEvent | EPOLLIN);
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