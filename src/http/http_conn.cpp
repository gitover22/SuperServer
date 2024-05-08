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

/**
 * @brief 初始化Http连接,初始化一个HTTP连接对象，包括设置文件描述符、客户端地址
 * @param fd [in] 文件描述符，用于网络通信
 * @param addr [in] 客户端地址信息
 */
void HttpConn::init(int fd,const sockaddr_in& addr){
    assert(fd>0);
    userCount++; // 增加用户连接计数
    this->addr = addr; // 设置客户端地址
    this->fd = fd; // 设置文件描述符
    writeBuff.RetrieveAll(); // 清空写缓冲区
    readBuff.RetrieveAll(); // 清空读缓冲区
    is_Close =false; // 标记连接为未关闭状态
    // 记录日志信息，包括文件描述符、客户端IP和端口、当前用户连接数
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd, GetIP(), GetPort(), (int)userCount);
}
/**
 * @brief 获取文件描述符
 * @return 文件描述符
 */
int HttpConn::GetFd() const {
    return fd;
}
/**
 * @brief 获取客户端地址信息
 * @return 客户端地址信息
 */
struct  sockaddr_in HttpConn::GetAddr() const{
    return addr;
}
/**
 * @brief 获取客户端IP地址
 * @return 客户端IP地址
 */
const char* HttpConn::GetIP()const {
    return inet_ntoa(addr.sin_addr);
}
/**
 * @brief 获取客户端端口号
 * @return 客户端端口号
 */
int HttpConn::GetPort() const{
    return addr.sin_port;
}

/**
 * 从HTTP连接中读缓冲区读取数据。
 * 
 * @param saveErrno 指向一个整型变量的指针，用于保存错误码。当读取过程中发生错误时，会将错误码保存到此处。
 * @return 返回读取到的数据的字节数。如果读取失败，返回-1。
 */
ssize_t HttpConn::read(int *saveErrno){
    ssize_t len =-1; // 初始化读取长度为-1，表示未读取到任何数据
    
    do{
        // 尝试从套接字读取数据到缓冲区
        len =readBuff.ReadFd(fd,saveErrno);
        // 如果读取的长度小于等于0，表示读取过程中遇到错误或连接已关闭
        if(len <=0){
            break;
        }
    // 如果启用了一种称为ET（边缘触发）的模式，将持续尝试读取数据，直到没有更多数据可读
    }while(isET);
    
    return len; // 返回读取到的数据长度
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
 int HttpConn::ToWriteBytes() { 
        return iov[0].iov_len + iov[1].iov_len; 
    }

bool HttpConn::IsKeepAlive() const {
    return request.IsKeepAlive();
}