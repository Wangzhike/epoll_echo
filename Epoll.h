//
// Created by zhikewang(王志科) on 2019-10-09.
//

#ifndef EPOLL_ECHO_EPOLL_H
#define EPOLL_ECHO_EPOLL_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include "common.h"

class Epoll {
public:
    Epoll(): _maxevents(-1), _epfd(-1), _events(nullptr) {
    }

    virtual ~Epoll() {
        if (_events != nullptr)
        {
            delete _events;
        }
        if (_epfd != -1)
        {
            close(_epfd);
        }
    }

    int create_epool(const int max_fd);
    int add_event(const int fd, const int state);
    int modify_event(const int fd, const int state);
    int del_event(const int fd, const int state);

    int wait(const int timeout_ms=5000);

    struct epoll_event& get(int i) {
        if (i > _maxevents) {
            LOG_PRINT("i: %d exceeds the maxevents: %d", i, _maxevents);
            exit(EXIT_FAILURE);
        }
        return _events[i];
    }

private:
    int _maxevents;
    int _epfd;
    struct epoll_event *_events;
};


#endif //EPOLL_ECHO_EPOLL_H
