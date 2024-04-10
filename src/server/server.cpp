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
Server::~Server(){
    close(listenFd);
    isClose =true;
    free(srcDir);
    SqlConnPool::Instance()->ClosePool(); // 关闭数据库连接池
}


void Server::Init_EventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;
    connEvent = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent |= EPOLLET;
        break;
    case 2:
        listenEvent |= EPOLLET;
        break;
    case 3:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    default:
        listenEvent |= EPOLLET;
        connEvent |= EPOLLET;
        break;
    }
    HttpConn::isET = (connEvent & EPOLLET);
}

void Server::Start() {
    int timeMS = -1;  /* epoll wait timeout == -1 无事件将阻塞 */
    if(!isClose) { LOG_INFO("========== Server start =========="); }
    while(!isClose) {
        if(_timeout > 0) {
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
    int ret = send(fd, info, strlen(info), 0);
    if(ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
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
    if(_timeout > 0) {
        timer->add(fd, _timeout, std::bind(&Server::Close_Conn, this, &users[fd]));
    }
    epoller->AddFd(fd, EPOLLIN | connEvent);
    Set_fd_Nonblock(fd);
    LOG_INFO("Client[%d] in!", users[fd].GetFd());
}
int Server::Set_fd_Nonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}