#ifndef EPOLL_ECHO_MEM_H
#define EPOLL_ECHO_MEM_H

int read_int(unsigned char *buffer) 
{
    return (int) (buffer[0] + (buffer[1] << 8) + (buffer[2] << 16) + (buffer[3] << 24));
}

int write_int(int val, char *a) 
{
    if(a == nullptr)
    {
        return -1;
    }

    a[0] = val & 0xff;
    a[1] = (val >>8)  & 0xff;
    a[2] = (val >>16) & 0xff;
    a[3] = (val >>24) & 0xff;  
    return 4;
}


#endif  //EPOLL_ECHO_MEM_H