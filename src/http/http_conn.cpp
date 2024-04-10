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

bool HttpConn::process() {
    request.Init();
    if(readBuff.ReadableBytes() <= 0) {
        return false;
    }
    else if(request.parse(readBuff)) {
        LOG_DEBUG("%s", request.path().c_str());
        response.Init(srcDir, request.path(), request.IsKeepAlive(), 200);
    } else {
        response.Init(srcDir, request.path(), false, 400);
    }
    response.MakeResponse(writeBuff);
    /* 响应头 */
    iov[0].iov_base = const_cast<char*>(writeBuff.Peek());
    iov[0].iov_len = writeBuff.ReadableBytes();
    iovCnt = 1;

    /* 文件 */
    if(response.FileLen() > 0  && response.File()) {
        iov[1].iov_base = response.File();
        iov[1].iov_len = response.FileLen();
        iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response.FileLen() , iovCnt, ToWriteBytes());
    return true;
}
