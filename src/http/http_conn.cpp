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

int HttpConn::GetPort() const{
    return addr.sin_port;
}

ssize_t HttpConn::read(int *saveErrno){
    ssize_t len =-1;
    do{
        len =readBuff.ReadFd(fd,saveErrno);
        if(len <=0){
            break;
        }
    }while(isET);
    return len;
}
ssize_t HttpConn::write(int* saveErrno){
    ssize_t len = -1;
    do{
        len =writev(fd,iov,iovCnt);
        if(len <=0){
            *saveErrno=errno;
            break;
        }
        if(iov[0].iov_len+iov[1].iov_len == 0) break;
        else if(static_cast<size_t>(len) > iov[0].iov_len){
            iov[1].iov_base = (uint8_t*) iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if(iov[0].iov_len) {
                writeBuff.RetrieveAll();
                iov[0].iov_len = 0;
            }
        }
        else {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len; 
            iov[0].iov_len -= len; 
            writeBuff.Retrieve(len);
        }
    }while(isET || ToWriteBytes() > 10240);
    return len;
}