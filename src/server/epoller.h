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
     * @brief 在当前的epoll实例（epoll_fd）上注册事件
     * @param fd [in] 待增加事件的文件描述符
     * @param events [in] fd对应的事件
     * @return 成功返回true，失败返回false
    */
    bool Add_Fd(int fd,uint32_t events);
    /**
     * @brief 修改当前epoll_fd实例上的事件
     * @param fd [in] 待修改事件的文件描述符
     * @param events [in] fd对应的事件
     * @return 成功返回true，失败返回false
    */
    bool Modify_Fd(int fd , uint32_t events);
    /**
     * @brief 从当前epoll_fd实例上(红黑树上)删除监听的事件
     * @param fd [in] 待删除事件的文件描述符
     * @return 成功返回true，失败返回false
    */
    bool Delete_Fd(int fd);
    /**
     * @brief 等待事件发生
     * @param wtime_ms [in] 等待时间，单位ms,默认值-1，无限等待
     * @return 成功返回有事件发生的事件数量，失败返回-1
    */
    int Wait(int wtime_ms = -1);
    /**
     * @brief 获取事件的文件描述符
     * @param i [in] 事件在__events中的下标
     * @return 返回事件的文件描述符
    */
    int GetEventFd(size_t i) const;
    /**
     * @brief 获取事件的事件类型
     * @param i [in] 事件在__events容器中的下标
     * @return 返回事件类型
    */
    uint32_t GetEvents(size_t i) const;
private:
    int epoll_fd; // epoll实体的对应的文件描述符
    std::vector<struct epoll_event> __events; // 记录epoll事件的容器

};

#endif