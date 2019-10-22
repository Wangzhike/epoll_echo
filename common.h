#ifndef EPOLL_ECHO_COMMON_H
#define EPOLL_ECHO_COMMON_H

#include <stdio.h>
#include <limits.h>

struct Buff {
    Buff(): _r_pos(_buf), _w_pos(_buf) {
    }

    Buff(const Buff& rhs) {
        for (int i = 0; i < LINE_MAX; ++i)
        {
            _buf[i] = rhs._buf[i];
        }

        _r_pos = _buf + (rhs._r_pos - rhs._buf);
        _w_pos = _buf + (rhs._w_pos - rhs._buf);
    }

    Buff& operator=(const Buff& rhs) {
        for (int i = 0; i < LINE_MAX; ++i)
        {
            _buf[i] = rhs._buf[i];
        }

        _r_pos = _buf + (rhs._r_pos - rhs._buf);
        _w_pos = _buf + (rhs._w_pos - rhs._buf);
        return *this;
    }
    
    size_t left_size() {
        return &_buf[LINE_MAX] - _r_pos;
    }

    size_t data_size() {
        return _r_pos - _w_pos;
    }

    void set_r_pow(int n) {
        _r_pos += n;
    }

    void set_w_pos(int n) {
        _w_pos += n;
        if (_r_pos == _w_pos) {
            _r_pos = _buf;
            _w_pos = _buf;
        }
    }

    char _buf[LINE_MAX];
    char *_r_pos;
    char *_w_pos;
};


#define __output(...) \
        fprintf(stderr, __VA_ARGS__); \
        fflush(stderr);
#define __format(__fmt__) "%s(%d)-<%s>: " __fmt__ "\n"
#define LOG_PRINT(__fmt__, ...) \
        __output(__format(__fmt__), __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#endif  //EPOLL_ECHO_COMMON_H