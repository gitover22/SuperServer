#ifndef EPOLLER_H
#define EPOLLER_H
#include <sys/epoll.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
class Epoller{
public:
    explicit Epoller(int maxEvent= 1024);
    ~Epoller();
    /**
     * @brief 在当前的epoll实例（epollFd_）上注册事件
     * @param fd [in] 待增加事件的文件描述符
     * @param events [in] 待增加的事件
    */
    bool AddFd(int fd,uint32_t events);
    bool ModFd(int fd , uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;
private:
    int epollFd_;
    std::vector<struct epoll_event> events_;

};

#endif