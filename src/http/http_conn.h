#ifndef HTTP_CONN_H
#define HTTP_CONN_H


#include <netinet/in.h>
#include "../buffer/buffer.h"
class HttpConn{
    HttpConn();

    ~HttpConn();

    void init(int sockFd,const sockaddr_in& addr);

    ssize_t read(int* saveErrno);

    ssize_t write(int* saveErrno);

    void Close();

    int GetFd() const;

    int GetPort() const;

    const char* GetIP() const;
    sockaddr_in GetAddr() const;
    bool process();

    int ToWriteBytes();

    bool IsKeepAlive() const;

    static bool isET;
    static const char* srcDir;
    static std::atomic<int> userCount;
private:
    int fd;
    struct sockaddr_in addr;
    bool is_Close;
    int iovCnt;
    struct iovec iov[2];

    Buffer readBuff;
    Buffer writeBuff; 
    HttpRequest request;
    HttpResponse response;

};

#endif