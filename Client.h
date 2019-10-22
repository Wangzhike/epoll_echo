#ifndef EPOLL_ECHO_CLIENT_H
#define EPOLL_ECHO_CLIENT_H

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#include "Epoll.h"
#include "common.h"


class Client {

public:
    Client(const std::string& ip, const int port):
        _epoll(), _sock_fd(-1), _svr_ip(ip), _svr_port(port), 
        _stdin2sock(), _sock2stdout(), _stdineof(false)
    {

    }
    bool conn_svr();
    void run();

private:
    int do_recv(int fd);
    int do_send(int fd);


private:
    Epoll _epoll;
    int _sock_fd;
    std::string _svr_ip;
    int _svr_port;
    Buff _stdin2sock;
    Buff _sock2stdout;
    
    bool _stdineof;

private:
    const static int kMax_conn_size = 4;
    const static int kWaitMS = 1000;
    const static int KRecvErr = -1;
    const static int kRecvDone = 0;
    const static int kRecvNotDone = 1;
    const static int err_clt_recv_normal = 0;
    const static int err_clt_recv_disconn = -1;
    const static int err_clt_recv_svrcrash = -2;
    const static int err_clt_recv_shutdown_wr = -3;
    const static int err_clt_recv_unknown_fd = -4;
    const static int err_clt_recv_error = -5;
    const static int err_clt_recv_nospace = -6;
    const static int err_clt_send_normal = 0;
    const static int err_clt_send_shutdown_rd = -7;
    const static int err_clt_send_unknown_fd = -8;
    const static int err_clt_send_error = -9;
};

#endif  //EPOLL_ECHO_CLIENT_H