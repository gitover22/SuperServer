#include "http_conn.h"


// 类的静态成员
bool HttpConn::isET;
const char* HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;
HttpConn::HttpConn(){
    fd = -1;
    addr={0};
    is_Close =true;
}

HttpConn::~HttpConn(){
    Close();
}
void HttpConn::Close(){
    response.UnmapFile();
    if(is_Close == false){
        is_Close =true;
        userCount--;
        close(fd);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd, GetIP(), GetPort(), (int)userCount);
    }
}

void HttpConn::init(int fd,const sockaddr_in& addr){
    assert(fd>0);
    userCount++;
    this->addr = addr;
    this->fd = fd;
    writeBuff.RetrieveAll();
    readBuff.RetrieveAll();
    is_Close =false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd, GetIP(), GetPort(), (int)userCount);

}

int HttpConn::GetFd() const {
    return fd;
}

struct  sockaddr_in HttpConn::GetAddr() const{
    return addr;
}
const char* HttpConn::GetIP()const {
    return inet_ntoa(addr.sin_addr);
}