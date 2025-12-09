#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

int handleRequest(
    char *buff,
    int i,
    int client_fd,
    int logged_in[],
    char client_user[][50]
);

#endif

