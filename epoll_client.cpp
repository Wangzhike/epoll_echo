#include <string>
#include "Client.h"

using namespace std;

int main()
{
    string svr_ip = "127.0.0.1";
    int svr_port = 9090;

    Client client(svr_ip, svr_port);

    if (client.conn_svr() == false)
    {
        LOG_PRINT("connect to server[%s:%d] failed!", svr_ip.c_str(), svr_port);
        return -1;
    }
    client.run();

    // int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    // fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    // const int MAX_EVENTS = 1024;
    // int epoll_fd;
    // epoll_fd = epoll_create(MAX_EVENTS);
    // if (epoll_fd < 0)
    // {
    //     perror("epoll_create failed");
    //     return -1;
    // }

    // struct epoll_event ev;
    // struct epoll_event events[MAX_EVENTS];
    
    // ev.data.fd = STDIN_FILENO;
    // ev.events = EPOLLIN;

    // int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &ev);
    // if (ret < 0)
    // {
    //     perror("epoll_ctl: stdin register failed");
    //     return -1;
    // }

    // while (1)
    // {
    //     int nfds;

    //     nfds = epoll_wait(epoll_fd, events, MAX_EVENTS, 10000);
    //     printf("loop events number: %d\n", nfds);

    //     for (int i = 0; i < nfds; ++i)
    //     {
    //         if (events[i].events & EPOLLIN)
    //         {
    //             if (events[i].data.fd == STDIN_FILENO)
    //             {
    //                 Buff buf;
    //                 int n = read(STDIN_FILENO, buf._r_pos, buf.left_size());
    //                 if (n < 0)
    //                 {
    //                     return -1;
    //                 }
    //                 buf.set_r_pow(n);
    //                 n = write(STDOUT_FILENO, buf._w_pos, buf.data_size());
    //                 if (n < 0)
    //                 {
    //                     return -1;
    //                 }
    //                 buf.set_w_pos(n);
    //             }
    //         }
    //     }
    // }

    return 0;
}