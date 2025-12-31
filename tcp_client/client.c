#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/time.h>

#include "network.h"
#include "explain_code.h"
#include "gameplay.h"
#include "menu.h"

#define MAXLINE 4096

//MAIN 
int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <ServerIP> <Port>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in servaddr;
    char sendBuff[MAXLINE], recvBuff[MAXLINE];
    char username[50];
    char password[50];
    int logged_in = 0;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    recvProcessor(sockfd, recvBuff, sizeof(recvBuff));
    explain_code(recvBuff);

    while (1) {
        int choice;
        menu(logged_in);
        scanf("%d", &choice);
        while (getchar() != '\n');

        flushSocket(sockfd);

        // NOT LOGGED IN
        if (!logged_in) {

            if (choice == 1) {
                printf("Username: "); scanf("%49s", username);
                printf("Password: "); scanf("%49s", password);
                snprintf(sendBuff, sizeof(sendBuff),
                         "REGISTER %s %s\n", username, password);
            }
            else if (choice == 2) {
                printf("Username: "); scanf("%49s", username);
                printf("Password: "); scanf("%49s", password);
                snprintf(sendBuff, sizeof(sendBuff),
                         "LOGIN %s %s\n", username, password);
            }
            else if (choice == 3) {
                printf("Exiting...\n");
                break;
            } else {
                printf("Invalid choice\n");
                continue;
            }

            send(sockfd, sendBuff, strlen(sendBuff), 0);
            recvProcessor(sockfd, recvBuff, sizeof(recvBuff));
            explain_code(recvBuff);

            if (strcmp(recvBuff,"110") == 0)
                logged_in = 1;
        }

        // LOGGED IN
        else {
            if (choice == 1) {
                send(sockfd, "JOIN\n", 5, 0);

                recvProcessor(sockfd, recvBuff, sizeof(recvBuff));
                explain_code(recvBuff);

                if (strcmp(recvBuff,"200") != 0)
                    continue;

                recvProcessor(sockfd, recvBuff, sizeof(recvBuff));
                explain_code(recvBuff);

                if (strcmp(recvBuff,"210") == 0) {
                    gamePlay(sockfd, username);
                }
            }
            else if (choice == 2) {
                send(sockfd, "LOGOUT\n", 7, 0);
                recvProcessor(sockfd, recvBuff, sizeof(recvBuff));
                explain_code(recvBuff);
                logged_in = 0;
            }
            else if (choice == 3) {
                printf("Exiting...\n");
                break;
            } else {
                printf("Invalid choice\n");
                continue;
            }
        }
    }

    close(sockfd);
    return 0;
}

