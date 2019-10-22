#include "Client.h"


bool Client::conn_svr()
{
    int sock_fd;
    
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG_PRINT("client sock creation failed, status %s", strerror(errno));
        return false;
    }
    _sock_fd = sock_fd;

    struct sockaddr_in client_addr;

    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = inet_addr(_svr_ip.c_str());
    client_addr.sin_port = htons(_svr_port);

    if (connect(_sock_fd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0)
    {
        LOG_PRINT("connect to server[%s:%d] failed", _svr_ip.c_str(), _svr_port);
        return false;
    }

    int flags = fcntl(_sock_fd, F_GETFL, 0);
    fcntl(_sock_fd, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(STDOUT_FILENO, F_GETFL, 0);
    fcntl(STDOUT_FILENO, F_SETFL, flags | O_NONBLOCK);

    _epoll.create_epool(kMax_conn_size);

    return true;
}

void Client::run()
{
    _epoll.add_event(_sock_fd, EPOLLIN);
    _epoll.add_event(STDIN_FILENO, EPOLLIN);

    
    while (true)
    {
        int nfds = _epoll.wait(kWaitMS);
        // LOG_PRINT("epoll wait get ready %d fds", nfds);

        for (int i = 0; i < nfds; ++i)
        {
            struct epoll_event &cur_ev = _epoll.get(i);

            if (cur_ev.events & EPOLLIN)
            {
                int cur_fd = cur_ev.data.fd;
                LOG_PRINT("epollin fd %d", cur_fd);
                int ret = do_recv(cur_fd);
                if (ret == err_clt_recv_normal)
                {
                    if (cur_fd == STDIN_FILENO)
                    {
                        _epoll.modify_event(_sock_fd, EPOLLOUT);
                    }
                    else if (cur_fd == _sock_fd)
                    {
                        _epoll.add_event(STDOUT_FILENO, EPOLLOUT);
                    }
                }
                else if (ret == err_clt_recv_disconn || ret == err_clt_recv_svrcrash)
                {
                    return ;
                }
                else if (ret == err_clt_recv_nospace)
                {
                    if (cur_fd == STDIN_FILENO && _stdin2sock.data_size() > 0)
                    {
                        _epoll.modify_event(_sock_fd, EPOLLOUT);
                    }
                    else if (cur_fd == _sock_fd && _sock2stdout.data_size() > 0)
                    {
                        int ret = _epoll.add_event(STDOUT_FILENO, EPOLLOUT);
                        if (ret < 0)
                        {
                            ret = _epoll.modify_event(STDOUT_FILENO, EPOLLOUT);
                            if (ret < 0)
                            {
                                LOG_PRINT("can't add or modify stdout to epoll_ctl");
                                exit(EXIT_FAILURE);
                            }
                        }
                    }
                }
            }
            else if (cur_ev.events & EPOLLOUT)
            {
                int cur_fd = cur_ev.data.fd;
                LOG_PRINT("epollout fd %d", cur_fd);

                int ret = do_send(cur_fd);

                if (ret == err_clt_send_shutdown_rd)
                {
                    return ;
                }
                else if (ret == err_clt_send_normal)
                {
                    if (cur_fd == STDOUT_FILENO)
                    {
                        _epoll.del_event(STDOUT_FILENO, EPOLLOUT);
                    }
                    else if (cur_fd == _sock_fd)
                    {
                        _epoll.modify_event(_sock_fd, EPOLLIN);
                    }
                }
            }
        }
    }
}

int Client::do_recv(int fd)
{
    int nrecv;

    if ((fd == _sock_fd && _sock2stdout.left_size() == 0) ||
        (fd == STDIN_FILENO && _stdin2sock.left_size() == 0))
    {
        if (fd == _sock_fd)
        {
            LOG_PRINT("sock_fd can read, but _sock2stdout have no empty space to read!");
        }
        else if (fd == STDIN_FILENO)
        {
            LOG_PRINT("stdin can read, buf _stdin2sock have no empty space to read!")
        }

        return err_clt_recv_nospace;
    }


    if (fd == _sock_fd)
    {
        nrecv = read(_sock_fd, _sock2stdout._r_pos, _sock2stdout.left_size());
    }
    else if (fd == STDIN_FILENO)
    {
        nrecv = read(STDIN_FILENO, _stdin2sock._r_pos, _stdin2sock.left_size());
    }
    else
    {
        LOG_PRINT("unknown fd: %d", fd);
        exit(err_clt_recv_unknown_fd);
    }

    if (nrecv < 0)
    {
        if (errno != EAGAIN)    //也就是 EWOULDBLOCK
        {
            LOG_PRINT("recv fd:%d occurs error, error %s", fd, strerror(errno));
            exit(err_clt_recv_error);

        }
    }
    else if (nrecv == 0)
    {
        if (fd == _sock_fd)
        {
            LOG_PRINT("read EOF on sock");
            if (_stdineof)
            {
                LOG_PRINT("EOF on sock after EOF on stdin, so disconnect to server[%s:%d]", _svr_ip.c_str(), _svr_port);
                return err_clt_recv_disconn;
            }
            else 
            {
                LOG_PRINT("server[%s:%d] terminated prematurely", _svr_ip.c_str(), _svr_port);
                exit(err_clt_recv_svrcrash);
            }
        }
        else if (fd == STDIN_FILENO)
        {
            LOG_PRINT("read EOF on stdin");
            _stdineof = true;
            if (_stdin2sock.data_size() == 0)
            {
                LOG_PRINT("EOF on stdin and no data to send sock, so shutdown sock write side");
                shutdown(_sock_fd, SHUT_WR);
                return err_clt_recv_shutdown_wr;
            }
        }
    }
    else
    {
        LOG_PRINT("read done from fd: %d, total nbytes: %d", fd, nrecv);
        if (fd == _sock_fd)
        {
            _sock2stdout.set_r_pow(nrecv);
        }
        else if (fd == STDIN_FILENO)
        {
            _stdin2sock.set_r_pow(nrecv);
        }
        return err_clt_recv_normal;
    }
    return err_clt_recv_normal;
}

int Client::do_send(int fd)
{
    int nsend = 0;
    if (fd == STDOUT_FILENO)
    {
        if (_sock2stdout.data_size() > 0)
        {
            nsend = write(fd, _sock2stdout._w_pos, _sock2stdout.data_size());
        }
    }
    else if (fd == _sock_fd)
    {
        if (_stdin2sock.data_size() > 0)
        {
            nsend = write(fd, _stdin2sock._w_pos, _stdin2sock.data_size());
        }
    }
    else
    {
        LOG_PRINT("unknown fd: %d", fd);
        exit(err_clt_send_unknown_fd);
    }

    if (nsend < 0)
    {
        if (errno !=EAGAIN)     //也就是 EWOULDBLOCK
        {
            LOG_PRINT("send fd: %d occurs error, error: %s", fd, strerror(errno));
            exit(err_clt_send_error);
        }
    }
    else
    {
        LOG_PRINT("send fd: %d done, total nbteys: %d", fd, nsend);
        if (fd == STDOUT_FILENO)
        {
            _sock2stdout.set_w_pos(nsend);
            if (_stdineof && _sock2stdout.data_size() == 0)
            {
                shutdown(_sock_fd, SHUT_RD);
                return err_clt_send_shutdown_rd;
            }
        }
        else if (fd == _sock_fd)
        {
            _stdin2sock.set_w_pos(nsend);
            return err_clt_send_normal;
        }
    }

    return err_clt_send_normal;
}

