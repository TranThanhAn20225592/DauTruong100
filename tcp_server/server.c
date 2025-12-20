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
#include <time.h>

#include "account.h"
#include "handle_request.h"
#include "join.h"
#include "player.h"
#include "question.h"
#include "game.h"

#define PORT 5550
#define BACKLOG 20
#define BUFF_SIZE 4096



ClientSession sessions[FD_SETSIZE];

int gameState = 0; // 0: dang cho, 1: dang choi game

// SERVER
int main() {
    int listenfd, connfd, sockfd;
    // int client[FD_SETSIZE];              // Mang luu socket cua cac client
    // int logged_in[FD_SETSIZE];           // Trang thai dang nhap cua moi client
    // char client_user[FD_SETSIZE][50];    // Luu username cua client
    fd_set allset, rset;                 // Tap socket theo doi
    int maxfd, maxi;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t clilen;
    char buff[BUFF_SIZE];

    // Tai du lieu khoi tao
    loadAccounts(); 
    loadQuestions("questions.txt");
    initPlayers();

    // Khoi tao danh sach client
    for (int i = 0; i < FD_SETSIZE; i++) {
        // client[i] = -1;
        // logged_in[i] = 0;
        // client_user[i][0] = 0;
        sessions[i].sockfd = -1;
        sessions[i].isLoggedIn = 0;
        sessions[i].username[0] = 0;
        sessions[i].bufferLen = 0;
    }

    // Tao socket cho server
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    // Cho phep tai su dung dia chi
    int opt = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Cau hinh dia chi server
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Gan socket voi dia chi
    if (bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Lang nghe ket noi tu client
    if (listen(listenfd, BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    // Khoi tao tap socket theo doi
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    maxfd = listenfd;
    maxi = -1;

    printf("SERVER started on port %d\n", PORT);

    while (1) {

        int timeoutCode = checkJoinTimeout();

        if (timeoutCode == 210) {
            printf("GAME START! Transferring players...\n");

            for (int j = 0; j < waitingCount; j++) {
                int sock = waitingRoom[j];
                char *name = NULL;

                for (int k = 0; k <= maxi; k++) {
                    if (sessions[k].sockfd == sock) {
                        name = sessions[k].username;
                        break;
                    }
                }

                if (name != NULL){
                    addPlayer(sock, name);
                    send(sock, "210\n", 4, 0);
                }
            }

            gameState = 1;
            initWaitingRoom();
            sleep(1);
            sendQuestionToAllPlayers(0);

        // Khong du nguoi choi -> huy phong cho
        } else if (timeoutCode == 202) {
            for (int j = 0; j < waitingCount; j++) {
                send(waitingRoom[j], "202\n", 4, 0);
            }
            initWaitingRoom();
        }

        rset = allset;

        // Thiet lap timeout cho select (1 giay)
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int nready = select(maxfd + 1, &rset, NULL, NULL, &tv);

        // Het thoi gian cho -> quay lai vong lap
        if (nready == 0) {
            continue;
        }

        if (nready < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        // Xu ly ket noi moi
        if (FD_ISSET(listenfd, &rset)) {
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen);
            if (connfd < 0) {
                perror("accept");
                continue;
            }

            int i;
            for (i = 0; i < FD_SETSIZE; i++) {
                if (sessions[i].sockfd < 0) {
                    sessions[i].sockfd = connfd;
                    sessions[i].addr = cliaddr;
                    sessions[i].isLoggedIn = 0;
                    sessions[i].bufferLen = 0;
                    memset(sessions[i].buffer, 0, BUFF_SIZE);
                    break;
                }
            }

            // Qua nhieu client
            if (i == FD_SETSIZE) {
                printf("Too many clients\n");
                close(connfd);
            } else {
                FD_SET(connfd, &allset);
                if (connfd > maxfd) maxfd = connfd;
                if (i > maxi) maxi = i;

                // Gui ma chao khi ket noi
                send(connfd, "900\n", 4, 0);
            }

            if (--nready <= 0) continue;
        }

        // Xu ly du lieu tu client
        for (int i = 0; i <= maxi; i++) {
            sockfd = sessions[i].sockfd;
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset)) {
                //int n = recv(sockfd, buff, BUFF_SIZE - 1, 0);
                int n;
                int freeSpace = BUFF_SIZE - 1 - sessions[i].bufferLen - 1;
                if (freeSpace <= 0) {
                    sessions[i].bufferLen = 0;
                    freeSpace = BUFF_SIZE - 1;
                }
                n = recv(sockfd, sessions[i].buffer + sessions[i].bufferLen, freeSpace, 0);
                if (n <= 0) {
                    printf("Client %d disconnected.\n", sockfd);
                    if (sessions[i].isLoggedIn) {
                        setUserOffline(sessions[i].username);
                        logoutAccount(sessions[i].username);
                        sessions[i].isLoggedIn = 0;
                        sessions[i].username[0] = 0;
                    }
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    sessions[i].sockfd = -1;
                    sessions[i].isLoggedIn = 0;
                    sessions[i].username[0] = 0;
                    sessions[i].bufferLen = 0;
                    sessions[i].buffer[0] = 0;
                }

                // Client ngat ket noi
                // if (n <= 0) {
                //     if (logged_in[i]) {
                //         setUserOffline(client_user[i]);
                //         logoutAccount(client_user[i]);
                //         logged_in[i] = 0;
                //         client_user[i][0] = 0;
                //     }
                //     close(sockfd);
                //     FD_CLR(sockfd, &allset);
                //     client[i] = -1;
                // } 
                else {
                    sessions[i].bufferLen += n;
                    sessions[i].buffer[sessions[i].bufferLen] = '\0';
                    char *lineStart = sessions[i].buffer;
                    char *lineEnd;
                    while ((lineEnd = strstr(lineStart, "\n")) != NULL) {
                        *lineEnd = '\0';
                        if (lineEnd > lineStart && *(lineEnd -1) == '\r') {
                            *(lineEnd -1) = '\0';
                        }
                        if (strlen(lineStart) > 0) {
                            printf("[DEBUG] Processing: %s\n", lineStart);
                            int code = handleRequest(lineStart, i, sockfd, sessions);
                            char out[16];
                            snprintf(out, sizeof(out), "%d\n", code);
                            send(sockfd, out, strlen(out), 0);
                        }
                        lineStart = lineEnd + 1;
                    }

                    // Xu ly yeu cau tu client
                    // int code = handleRequest(buff, i, sockfd, logged_in, client_user);

                    // Gui ma phan hoi
                    // char out[16];
                    // snprintf(out, sizeof(out), "%d", code);
                    // send(sockfd, out, strlen(out), 0);
                    int remainingLen = sessions[i].bufferLen - (lineStart - sessions[i].buffer);
                    if (remainingLen > 0) {
                        memmove(sessions[i].buffer, lineStart, remainingLen);
                    }
                    sessions[i].bufferLen = remainingLen;
                    memset(sessions[i].buffer + sessions[i].bufferLen, 0, BUFF_SIZE - sessions[i].bufferLen);
                }

                if (--nready <= 0) break;
            }
        }
    }

    // Dong tat ca socket
    for (int i = 0; i <= maxi; i++)
        if (sessions[i].sockfd > 0) close(sessions[i].sockfd);

    close(listenfd);
    return 0;
}





