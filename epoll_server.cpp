#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

#include "Conn.h"

using namespace std;

const static int MaxFdSize = 1024;
const static int ServerPort = 9090;

int main()
{
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        LOG_PRINT("create socket failed! err: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(ServerPort);

    int ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret != 0)
    {
        LOG_PRINT("bind sock failed, err: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }
    ret = listen(sock_fd, MaxFdSize);
    if (ret != 0)
    {
        LOG_PRINT("listen sock failed! err: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    Conn conn;
    conn.init(sock_fd, MaxFdSize);
    conn.run();

    close(sock_fd);
    LOG_PRINT("server exit, status: %s", strerror(errno));

    return 0;
}