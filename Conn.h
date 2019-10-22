#ifndef EPOLL_ECHO_CONN_H
#define EPOLL_ECHO_CONN_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <limits.h>

#include <string>
#include <map>
#include "Epoll.h"
#include "common.h"

struct ConnInfo {
    int fd;
    std::string ip;
    uint16_t port;
    time_t accpet_time;
    Buff buf;
    bool readeof;

    ConnInfo(): fd(-1), port(-1), buf(), readeof(false) {
    }
};

class Conn {
public:
    Conn(): _epoll(), _listen_fd(-1), _cur_conn_size(0), _max_conn_size(0), _conn_info_map(), _cur_handler(0)
    {
    }

    bool init(const int listen_fd, const int max_conn_size=1024);
    void run();

private:
    //void mod_fd(const int fd, int state);
    void handle(struct epoll_event &cur_ev);
    void add_conn(const int fd, const struct sockaddr_in& client_addr);
    void del_conn(const int fd);
    ConnInfo *get_conn(const int fd);

    int do_send(ConnInfo *conn_info);
    int do_recv(ConnInfo *conn_info);

private:
    Epoll _epoll;
    int _listen_fd;
    int _cur_conn_size;
    int _max_conn_size;
    std::map<int, ConnInfo> _conn_info_map;
    int _cur_handler;

    const static int kWaitMS = 1000;
    const static int kRecvErr = -1;
    const static int kRecvDone = 0;
    const static int kRecvNotDone = 1;
    const static int err_svr_recv_normal = 0;
    const static int err_svr_recv_eof = -1;
    const static int err_svr_recv_error = -2;
    const static int err_svr_recv_nospace = -3;
    const static int err_svr_send_normal = 0;
    const static int err_svr_send_disconn = -4;
    const static int err_svr_send_error = -5;
};

#endif  //EPOLL_ECHO_CONN_H