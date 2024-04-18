#ifndef HTTP_CONN_H
#define HTTP_CONN_H


#include <netinet/in.h>
#include <arpa/inet.h>   // sockaddr_in
#include "../buffer/buffer.h"
#include "./http_request.h"
#include "./http_response.h"
#include "../log/log.h"

class HttpConn{
public:
    HttpConn();  // 构造函数
    ~HttpConn();  // 析构函数

    void init(int sockFd, const sockaddr_in& addr);  // 初始化连接

    ssize_t read(int* saveErrno);  // 从socket读数据
    ssize_t write(int* saveErrno);  // 向socket写数据

    void Close();  // 关闭连接

    int GetFd() const;  // 获取文件描述符
    int GetPort() const;  // 获取端口号
    const char* GetIP() const;  // 获取IP地址
    sockaddr_in GetAddr() const;  // 获取sockaddr_in结构
    bool process();  // 处理读取的数据

    int ToWriteBytes();  // 计算待写入的字节数
    bool IsKeepAlive() const;  // 检查连接是否保持活跃

    static bool isET;  // 使用边缘触发模式
    static const char* srcDir;  // 资源目录
    static std::atomic<int> userCount;  // 用户计数

private:
    int fd;  // 文件描述符
    struct sockaddr_in addr;  // 客户端地址
    bool is_Close;  // 连接是否已关闭
    int iovCnt;  // iovec数组的元素数
    struct iovec iov[2];  // 用于写操作的iovec数组
    Buffer readBuff;  // 读缓冲区
    Buffer writeBuff;  // 写缓冲区
    HttpRequest request;  // HTTP请求对象
    HttpResponse response;  // HTTP响应对象
};
#endif