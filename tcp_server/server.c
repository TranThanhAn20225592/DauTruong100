#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

#include "account.h"

#define PORT 5550
#define BACKLOG 20
#define BUFF_SIZE 4096

//  DANH S�CH USER �ANG ONLINE
char onlineUser[1000][50];
int onlineCount = 0;

void writeLog(int functionId, const char *value, const char *result) {
    FILE *f = fopen("log.txt", "a");
    if (!f) return;

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (!t) {
        fclose(f);
        return;
    }

    fprintf(f, "[%02d/%02d/%04d %02d:%02d:%02d] $ %d $ %s $ %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec, functionId, value ? value : "", result ? result : "");
    fclose(f);
}

int isUserOnline(char *username) {
    for (int i = 0; i < onlineCount; i++) {
        if (strcmp(onlineUser[i], username) == 0)
            return 1;
    }
    return 0;
}

void setUserOnline(char *username) {
    strcpy(onlineUser[onlineCount], username);
    onlineCount++;
}

void setUserOffline(char *username) {
    for (int i = 0; i < onlineCount; i++) {
        if (strcmp(onlineUser[i], username) == 0) {
            onlineCount--;
            strcpy(onlineUser[i], onlineUser[onlineCount]);
            return;
        }
    }
}

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
        rset = allset;
        int nready = select(maxfd+1, &rset, NULL, NULL, NULL);
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
                send(connfd, "100", 3, 0); // hello code
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
                    char payload[50];
                    char cmd[64], u[50], p[50];
                    cmd[0] = u[0] = p[0] = 0;
                    int cnt = sscanf(buff, "%s %s %s", cmd, u, p);

                    int code = 199;

                    // REGISTER
                    if (cnt >= 1 && strcmp(cmd,"REGISTER")==0) {
                        if (cnt != 3) {
                            code = 199;
                            writeLog(1, "", "-ERR 119");
                        }
                        else {
                            code = registerAccount(u,p);
                            strcpy(payload, u);
                            if (code == 100) {
                                writeLog(1, payload, "+OK 110");
                            } else {
                                writeLog(1, payload, "-ERR 101");
                            }
                        }
                    } 
                    
                    // LOGIN
                    else if (cnt >=1 && strcmp(cmd,"LOGIN")==0) {

                        if (cnt != 3) code = 199;
                        else {

                            if (isUserOnline(u)) {
                                code = 113;
                                strcpy(payload, u);
                                writeLog(2, payload, "-ERR 113");
                            } else {
                                code = loginAccount(u,p);
                                if (code == 110) {
                                    logged_in[i] = 1;
                                    strcpy(client_user[i], u);
                                    setUserOnline(u);
                                    strcpy(payload, u);
                                    writeLog(2, payload, "+OK 110");
                                }
                            }
                        }
                    }

                    // LOGOUT
                    else if (cnt >= 1 && strcmp(cmd,"LOGOUT")==0) {

                        if (!logged_in[i]) {
                            code = 121;
                            writeLog(3, "", "-ERR 121");
                        }
                        else {
                            code = logoutAccount(client_user[i]);
                            if (code == 120) {
                                setUserOffline(client_user[i]);
                                logged_in[i] = 0;
                                client_user[i][0] = 0;
                                strcpy(payload, client_user[i]);
                                writeLog(3, payload, "+OK 120");
                            }
                        }
                    }

                    // Error 
                    else {
                        code = 199;
                    }

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

