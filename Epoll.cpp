//
// Created by zhikewang(王志科) on 2019-10-09.
//

#include "Epoll.h"

int Epoll::create_epool(const int max_fd)
{
    _maxevents = max_fd;
    _events = new epoll_event[max_fd];
    _epfd = epoll_create(max_fd+1);
    if (_epfd < 0)
    {
        LOG_PRINT("epoll_create failed! error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    return 0;
}

int Epoll::add_event(const int fd, const int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    int ret = epoll_ctl(_epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0)
    {
        LOG_PRINT("epoll_ctl register fd: %d failed! error: %s", fd, strerror(errno));
        // exit(EXIT_FAILURE);
        return -1;
    }

    return 0;
}

int Epoll::modify_event(const int fd, const int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    int ret = epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &ev);
    if (ret < 0)
    {
        LOG_PRINT("epoll_ctl modify fd:%d failed! error: %s", fd, strerror(errno));
        // exit(EXIT_FAILURE);
        return -1;
    }

    return 0;
}

int Epoll::del_event(const int fd, const int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    int ret = epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, &ev);
    if (ret < 0)
    {
        LOG_PRINT("epoll_ctl delete fd: %d failed! error: %s", fd, strerror(errno));
        // exit(EXIT_FAILURE);
        return -1;
    }

    return 0;
}

int Epoll::wait(const int timeout_ms)
{
    int nfds = epoll_wait(_epfd, _events, _maxevents, timeout_ms);

    return nfds;
}
