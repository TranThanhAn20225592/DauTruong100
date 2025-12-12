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

int gameState = 0; // 0: dang cho, 1: dang choi game

// SERVER
int main() {
    int listenfd, connfd, sockfd;
    int client[FD_SETSIZE];              // Mang luu socket cua cac client
    int logged_in[FD_SETSIZE];           // Trang thai dang nhap cua moi client
    char client_user[FD_SETSIZE][50];    // Luu username cua client
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
        client[i] = -1;
        logged_in[i] = 0;
        client_user[i][0] = 0;
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

        // Kiem tra thoi gian cho trong phong cho JOIN
        int timeoutCode = checkJoinTimeout();

        // Du nguoi choi -> bat dau game
        if (timeoutCode == 210) {
            printf("GAME START! Transferring players...\n");

            // Gui ma bat dau cho tat ca client trong phong cho
            for (int j = 0; j < waitingCount; j++) {
                int sock = waitingRoom[j];
                char *name = "Unknown";

                // Tim ten nguoi choi tu danh sach client
                for (int k = 0; k <= maxi; k++) {
                    if (client[k] == sock) {
                        name = client_user[k];
                        break;
                    }
                }

                // Them nguoi choi vao danh sach thi dau
                addPlayer(sock, name);

                // Gui ma bat dau game
                send(sock, "210", 4, 0);
            }

            // Chuyen trang thai sang dang choi
            gameState = 1;
            initWaitingRoom();

            sleep(1);

            // Gui cau hoi dau tien cho tat ca nguoi choi
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
                if (client[i] < 0) {
                    client[i] = connfd;
                    logged_in[i] = 0;
                    client_user[i][0] = 0;
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
                send(connfd, "900", 3, 0);
            }

            if (--nready <= 0) continue;
        }

        // Xu ly du lieu tu client
        for (int i = 0; i <= maxi; i++) {
            sockfd = client[i];
            if (sockfd < 0) continue;

            if (FD_ISSET(sockfd, &rset)) {
                int n = recv(sockfd, buff, BUFF_SIZE - 1, 0);

                // Client ngat ket noi
                if (n <= 0) {
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

                    // Xu ly yeu cau tu client
                    int code = handleRequest(buff, i, sockfd, logged_in, client_user);

                    // Gui ma phan hoi
                    char out[16];
                    snprintf(out, sizeof(out), "%d", code);
                    send(sockfd, out, strlen(out), 0);
                }

                if (--nready <= 0) break;
            }
        }
    }

    // Dong tat ca socket
    for (int i = 0; i <= maxi; i++)
        if (client[i] > 0) close(client[i]);

    close(listenfd);
    return 0;
}





