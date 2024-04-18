#include "epoller.h"
Epoller::Epoller(int maxEvent):epoll_fd(epoll_create(512)),__events(maxEvent){
    assert(epoll_fd>=0 && __events.size() > 0);
}
Epoller::~Epoller(){
    close(epoll_fd);
}
bool Epoller::AddFd(int fd,uint32_t events){
    if(fd < 0 ) return false;
    epoll_event ev = {0};
    ev.data.fd =fd;
    ev.events = events;
    return 0==epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&ev);
}
bool Epoller::ModFd(int fd , uint32_t events){
    if(fd<0) return false;
    epoll_event ev ={0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);
}
bool Epoller::DelFd(int fd){
    if(fd<0) return false;
    epoll_event ev ={0};
    return 0 == epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,&ev);

}
int Epoller::Wait(int timeout){
    return epoll_wait(epoll_fd,&__events[0],static_cast<int>(__events.size()),timeout);
}
int Epoller::GetEventFd(size_t i) const{
    assert(i<__events.size() && i>=0);
    return __events[i].data.fd;
}
uint32_t Epoller::GetEvents(size_t i) const{
    assert(i<__events.size() && i>=0);
    return __events[i].events;
}