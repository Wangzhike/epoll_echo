#include "Conn.h"
#include "mem.h"

bool Conn::init(const int listen_fd, const int max_conn_size)
{
    _listen_fd = listen_fd;
    _cur_conn_size = 0;
    _max_conn_size = max_conn_size;

    _epoll.create_epool(_max_conn_size);

    _cur_handler = 0;

    return true;
}

void Conn::run()
{
    _epoll.add_event(_listen_fd, EPOLLIN);

    while (true)
    {
        int nfds = _epoll.wait(kWaitMS);
        // LOG_PRINT("epoll wait get ready %d fds", nfds);

        for (int i = 0; i < nfds; ++i)
        {
            struct epoll_event &cur_ev = _epoll.get(i);
            handle(cur_ev);
        }
    }
}

void Conn::handle(struct epoll_event &cur_ev)
{
    if (cur_ev.events & EPOLLIN)
    {
        int cur_fd = cur_ev.data.fd;
        if (cur_fd != _listen_fd)   //sockfd
        {
            LOG_PRINT("epollin sockfd %d", cur_fd);
            ConnInfo *conn_info = get_conn(cur_fd);
            if (conn_info != nullptr)
            {
                int ret = do_recv(conn_info);
                if (ret == err_svr_recv_normal)
                {
                    //修改sockfd描述符事件，增加写事件
                    _epoll.modify_event(cur_fd, EPOLLOUT);
                }
                else if (ret == err_svr_recv_eof)
                {
                    if ((conn_info->buf).data_size() == 0)
                    {
                        del_conn(conn_info->fd);
                    }
                    else
                    {
                        //修改sockfd描述符事件，增加写事件
                        _epoll.modify_event(cur_fd, EPOLLOUT);
                    }
                }
                else if (ret == err_svr_recv_nospace)
                {
                    // Buff& buf = conn_info->buf;
                    // if (buf.data_size() > 0)
                    // {
                    //     //修改sockfd描述符事件，增加写事件
                    //     _epoll.modify_event(cur_fd, EPOLLOUT);
                    // }
                }
            }
            else
            {
                LOG_PRINT("Error: sockfd %d don't have conn info", cur_fd);
                return ;
            }  
        }
        else    //listen_fd
        {
            struct sockaddr_in client_sock_addr;
            socklen_t client_sock_len;
            client_sock_len = sizeof(struct sockaddr_in);
            int fd = accept(_listen_fd, (struct sockaddr *)&client_sock_addr, &client_sock_len);
            if (fd < 0)
            {
                LOG_PRINT("accept sock failed!");
                return ;
            }
            LOG_PRINT("accept fd %d, ip: %s port: %d", fd, inet_ntoa(client_sock_addr.sin_addr), ntohs(client_sock_addr.sin_port));
            add_conn(fd, client_sock_addr);

            int flags = fcntl(fd, F_GETFL, 0);
            fcntl(fd, F_SETFL, flags | O_NONBLOCK);
            _epoll.add_event(fd, EPOLLIN);
        }
    }
    else if (cur_ev.events & EPOLLOUT)
    {
        int cur_fd = cur_ev.data.fd;
        LOG_PRINT("epollout fd %d", cur_fd);
        ConnInfo *conn_info = get_conn(cur_fd);
        int ret = do_send(conn_info);
        if (ret == err_svr_send_disconn)
        {
            del_conn(cur_fd);
        }
        else if (ret == err_svr_send_normal)
        {
            _epoll.modify_event(cur_fd, EPOLLIN);
        }
        
    }
    else if (cur_ev.events & EPOLLHUP)
    {
        LOG_PRINT("epollhup fd %d", cur_ev.data.fd);
    }
}

int Conn::do_recv(ConnInfo *conn_info)
{
    Buff &buf = conn_info->buf;
    // LOG_PRINT("conn_info->buf: %p, conn_info->buf.left_size(): %lu", buf._buf, buf.left_size());
    if (buf.left_size() == 0)
    {
        LOG_PRINT("sock_fd: %d can read, but buf have no space to read!", conn_info->fd);
        return err_svr_recv_nospace;
    }

    int nrecv = read(conn_info->fd, buf._r_pos, buf.left_size());
    if (nrecv < 0)
    {
        if (errno != EAGAIN)    //也就是 EWOULDBLOCK
        {
            LOG_PRINT("recv fd %d occurs error, error %s", conn_info->fd, strerror(errno));
            exit(err_svr_recv_error);
        }
    }
    else if (nrecv == 0)
    {
        LOG_PRINT("recv fd %d, close by client", conn_info->fd);
        conn_info->readeof = true;
        return err_svr_recv_eof;
    }
    else
    {
        LOG_PRINT("read done from fd: %d, total nbytes: %d", conn_info->fd, nrecv);
        buf.set_r_pow(nrecv);
        return err_svr_recv_normal;
    }

    return err_svr_recv_normal;
}

int Conn::do_send(ConnInfo *conn_info)
{
    Buff &buf = conn_info->buf;
    int nsend = 0;

    if (buf.data_size() > 0)
    {
        nsend = write(conn_info->fd, buf._w_pos, buf.data_size());
    }
    
    if (nsend < 0)
    {
        if (errno != EAGAIN)
        {
            LOG_PRINT("send fd: %d occurs error, error: %s", conn_info->fd, strerror(errno));
            exit(err_svr_send_error);
        }
    }
    else
    {
        LOG_PRINT("send fd: %d done, total nbytes: %d", conn_info->fd, nsend);
        buf.set_w_pos(nsend);
        if (conn_info->readeof && buf.data_size() == 0)
        {
            return err_svr_send_disconn;
        }
    }

    return err_svr_send_normal;
}

void Conn::add_conn(const int fd, const struct sockaddr_in& client_addr)
{
    LOG_PRINT("cur fd size %lu", _conn_info_map.size());
    if (_conn_info_map.size() > (unsigned int)_max_conn_size)
    {
        LOG_PRINT("out max fd size");
        close(fd);
        return ;
    }

    ConnInfo conn_info;
    conn_info.fd = fd;
    conn_info.ip = std::string(inet_ntoa(client_addr.sin_addr));
    conn_info.port = ntohs(client_addr.sin_port);
    conn_info.readeof = false;
    time(&(conn_info.accpet_time));
    _conn_info_map[fd] = conn_info;
}

void Conn::del_conn(const int fd)
{
    LOG_PRINT("cur conn fd size %lu", _conn_info_map.size());
    if (_conn_info_map.size() < 0)
    {
        LOG_PRINT("have no fds");
        return ;
    }

    std::map<int, ConnInfo>::iterator it = _conn_info_map.find(fd);
    if (it != _conn_info_map.end())
    {
        _conn_info_map.erase(it);
    }

    _epoll.del_event(fd, EPOLLIN|EPOLLOUT);
    close(fd);
}

ConnInfo *Conn::get_conn(const int fd)
{
    std::map<int, ConnInfo>::iterator it = _conn_info_map.find(fd);
    if (it != _conn_info_map.end())
    {
        return &(it->second);
    }

    return nullptr;
}