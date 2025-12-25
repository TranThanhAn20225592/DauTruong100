#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <netinet/in.h>

#define BUFF_SIZE 4096

typedef struct {
    int sockfd;
    struct sockaddr_in addr;    // Luu socket cua client
    char username[50];          // Luu username cua client
    int isLoggedIn;             // Trang thai dang nhap cua client
    char buffer[BUFF_SIZE];
    int bufferLen;
    int pendingRoundEnd;
} ClientSession;

int handleRequest(
    char *buff,
    int i,
    int client_fd,
    ClientSession *sessions
);

void handleClientDisconnect(
    int client_fd,
    int idx,
    ClientSession *sessions
);

#endif

