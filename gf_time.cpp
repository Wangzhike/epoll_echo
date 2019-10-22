#include <time.h>
#include <sys/time.h>

#include "common.h"

const char *gf_time(void)
{
    struct timeval tv;
    static char str[30];
    char *ptr;

    if (gettimeofday(&tv, nullptr) < 0)
    {
        LOG_PRINT("gettimeofday error");
        return nullptr;
    }

    ptr = ctime(&tv.tv_sec);
    strcpy(str, &ptr[11]);

    snprintf(str+8, sizeof(str)-8, ".%06ld", tv.tv_usec);

    return str;
}