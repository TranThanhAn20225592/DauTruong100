#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "account.h"
#include "handle_request.h"
#include "join.h"

#define PORT 5550
#define BACKLOG 20
#define BUFF_SIZE 4096

//SERVER
int main() {
    int listenfd, connfd, sockfd;
    int client[FD_SETSIZE];
    int logged_in[FD_SETSIZE];
    char client_user[FD_SETSIZE][50];
    fd_set allset, rset;
    int maxfd, maxi;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    char buff[BUFF_SIZE];

    loadAccounts(); 

    for (int i = 0; i < FD_SETSIZE; i++) {
        client[i] = -1;
        logged_in[i] = 0;
        client_user[i][0] = 0;
    }

    // create socket  
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    printf("SERVER started on port %d\n", PORT);

    while (1) {
    	// KIEM TRA TIMEOUT PHÒNG CHO JOIN
        int timeoutCode = checkJoinTimeout();
        if (timeoutCode == 210 || timeoutCode == 202) {

          // Gui mã cho tat ca client trong phòng cho
          for (int j = 0; j < waitingCount; j++) {
            char notify[16];
            sprintf(notify, "%d", timeoutCode);
            send(waitingRoom[j], notify, strlen(notify), 0);
          }

          // Reset phòng sau khi gui mã
          initWaitingRoom();
        }
    	
        rset = allset;

        // Tao timeout cho select (1 giây)
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int nready = select(maxfd+1, &rset, NULL, NULL, &tv);

        // Neu het thoi gian cho -> quay lai vòng lap de kiem tra timeout JOIN
        if (nready == 0) {
          continue;
        }

        if (nready < 0) {
          if (errno == EINTR) continue;
          perror("select");
          break;
        }


        // New connection 
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
            if (connfd < 0) {
                perror("accept");
                continue;
            }

            int i;
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    logged_in[i] = 0;
                    client_user[i][0] = 0;
                    break;
                }
            }

            if (i == FD_SETSIZE) {
                printf("Too many clients\n");
                close(connfd);
            } else {
                FD_SET(connfd, &allset);
                if (connfd > maxfd) maxfd = connfd;
                if (i > maxi) maxi = i;
                send(connfd, "900", 3, 0); // hello code
            }

            if (--nready <= 0) continue;
        }

        // DATA CLIENT
        for (int i = 0; i <= maxi; i++) {
            sockfd = client[i];
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset)) {

                int n = recv(sockfd, buff, BUFF_SIZE-1, 0);
                if (n <= 0) {
                    // Client not connect 
                    if (logged_in[i]) {
                        setUserOffline(client_user[i]);   
                        logoutAccount(client_user[i]);
                        logged_in[i] = 0;
                        client_user[i][0] = 0;
                    }
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                } else {
                    buff[n] = 0;
                    int code = handleRequest(buff, i, sockfd, logged_in, client_user);
                    char out[16];
                    snprintf(out, sizeof(out), "%d", code);
                    send(sockfd, out, strlen(out), 0);
                }

                if (--nready <= 0) break;
            }
        }
    }

    for (int i = 0; i <= maxi; i++)
        if (client[i] > 0) close(client[i]);

    close(listenfd);
    return 0;
}

