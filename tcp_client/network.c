#include "network.h"
#include <sys/socket.h>
#include <unistd.h>

void flushSocket(int sockfd) {
    char dump[1024];
    while (recv(sockfd, dump, sizeof(dump), MSG_DONTWAIT) > 0) {}
}

int recvProcessor(int sockfd, char *buffer, int bufferSize) {
    int n = recv(sockfd, buffer, bufferSize - 1, 0);
    if (n > 0) {
        buffer[n] = '\0';
        if (buffer[n-1] == '\n') buffer[n-1] = '\0';
        if (n > 1 && buffer[n-2] == '\r') buffer[n-2] = '\0';
    }
    return n;
}
