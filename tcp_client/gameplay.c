#include "gameplay.h"
#include "explain_code.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define MAXLINE 4096

//GAME PLAY
void gamePlay(int sockfd, const char *my_username) {
    char recvBuff[MAXLINE];
    char sendBuff[MAXLINE];

    int skipLeft = -1;
    int sentAnswer = 0;   // flag sent answer

    while (1) {

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(sockfd, &fds);
        FD_SET(0, &fds);   // stdin

        int maxfd = sockfd > 0 ? sockfd : 0;

        if (select(maxfd + 1, &fds, NULL, NULL, NULL) < 0) {
            perror("select");
            return;
        }

        // SERVER MESSAGE
        if (FD_ISSET(sockfd, &fds)) {
            int n = recv(sockfd, recvBuff, sizeof(recvBuff) - 1, 0);
            if (n <= 0) {
                puts("Disconnected from server!");
                return;
            }
            recvBuff[n] = 0;

            char *saveptr;
            char *line = strtok_r(recvBuff, "\n", &saveptr);

            while (line) {

                // SKIP INFO
                if (strncmp(line, "SKIP_INFO|", 10) == 0) {
                    char tmp[MAXLINE];
                    strncpy(tmp, line, sizeof(tmp));
                    strtok(tmp, "|");
                    char *countStr = strtok(NULL, "|");
                    if (countStr) skipLeft = atoi(countStr);
                }

                // QUESTION
                else if (strncmp(line, "QUES|", 5) == 0) {

                    sentAnswer = 0;   // reset answer for new question

                    char tmp[MAXLINE];
                    strncpy(tmp, line, sizeof(tmp));
                    tmp[sizeof(tmp) - 1] = 0;

                    strtok(tmp, "|");
                    char *question = strtok(NULL, "|");
                    char *op1 = strtok(NULL, "|");
                    char *op2 = strtok(NULL, "|");
                    char *op3 = strtok(NULL, "|");
                    char *op4 = strtok(NULL, "|");

                    printf("\n[QUESTION]: %s\n", question);
                    printf("1. %s\n", op1);
                    printf("2. %s\n", op2);
                    printf("3. %s\n", op3);
                    printf("4. %s\n", op4);

                    if (skipLeft >= 0) {
                        printf("\n---------------------------------------------\n");
                        if (skipLeft > 0)
                            printf("(!) You have %d skips left (enter 5 to skip)\n", skipLeft);
                        else
                            printf("(!) No skips left (5 = eliminated)\n");
                        printf("---------------------------------------------\n");
                    }

                    printf("Enter your answer (1-4");
                    if (skipLeft >= 0) printf(", 5=SKIP");
                    printf("): ");
                    fflush(stdout);
                }

                // STAGE 1 RESULT
                else if (strncmp(line, "RESULT|", 7) == 0) {
                    char tmp[MAXLINE];
                    strncpy(tmp, line, sizeof(tmp));
                    strtok(tmp, "|");
                    char *winner = strtok(NULL, "|");

                    if (winner && strcmp(winner, "NONE") == 0)
                        puts("No winner for this question.");
                    else if (winner && strcmp(winner, my_username) == 0)
                        puts("You are the MAIN player!");
                    else
                        puts("You are a SUB player.");
                }

                // SCORE
                else if (strncmp(line, "SCORE|YOU|", 10) == 0) {
                    strtok(line, "|");
                    strtok(NULL, "|");
                    char *score = strtok(NULL, "|");
                    printf("Your score: %s\n", score);
                }

                // LOG 
                else if (strncmp(line, "LOG|", 4) == 0) {
                    printf("[SPECTATOR] %s\n", line + 4);
                }

                // RESULT / ACK CODES 
                else if (isdigit((unsigned char)line[0])) {

                    // print 300 if player sent answer
                    if (strcmp(line, "300") == 0) {
                        if (sentAnswer) {
                            explain_code(line);   // Answer received
                        }
                        sentAnswer = 0;  // reset sau ACK
                    }
                    else {
                        explain_code(line);
                    }

                    if (strcmp(line, "410") == 0) {
                        puts("You are eliminated. Watching the game...");
                    }
                    else if (strcmp(line, "420") == 0 ||
                             strcmp(line, "421") == 0) {
                        return;
                    }
                }

                line = strtok_r(NULL, "\n", &saveptr);
            }
        }

        // USER INPUT
        if (FD_ISSET(0, &fds)) {
            int ans;
            if (scanf("%d", &ans) == 1) {
                while (getchar() != '\n');

                if (ans == 5) {
                    send(sockfd, "SKIP\n", 5, 0);
                } else {
                    snprintf(sendBuff, sizeof(sendBuff),
                             "ANSWER %d\n", ans);
                    send(sockfd, sendBuff, strlen(sendBuff), 0);
                }

                sentAnswer = 1;   // answer sent
            }
        }
    }
}

