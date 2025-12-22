#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXLINE 4096

void explain_code(const char *code) {
    if (strcmp(code,"900") == 0) puts("Connected to server");
    else if (strcmp(code,"100") == 0) puts("Register successful");
    else if (strcmp(code,"101") == 0) puts("Account already exists");
    else if (strcmp(code,"110") == 0) puts("Login successful");
    else if (strcmp(code,"111") == 0) puts("Wrong password");
    else if (strcmp(code,"112") == 0) puts("Account does not exist");
    else if (strcmp(code,"113") == 0) puts("Account already logged in");
    else if (strcmp(code,"120") == 0) puts("Logout successful");
    else if (strcmp(code,"121") == 0) puts("Logout failed");

    else if (strcmp(code,"200") == 0) puts("JOIN OK - Waiting for other players...");
    else if (strcmp(code,"201") == 0) puts("Player limit exceeded!");
    else if (strcmp(code,"202") == 0) puts("Insufficient number of player!");
    else if (strcmp(code,"203") == 0) puts("Cannot JOIN during an ongoing game!");
    else if (strcmp(code,"210") == 0) puts("GAME START!");

    else if (strcmp(code,"300") == 0) puts("Answer received");
    else if (strcmp(code,"301") == 0) puts("You are not in the game");
    else if (strcmp(code,"302") == 0) puts("You have already answered");
    
    else if (strcmp(code,"400") == 0) puts("Answer correct");
    else if (strcmp(code,"401") == 0) puts("Answer wrong");
    else if (strcmp(code,"402") == 0) puts("Answer timeout");
    else if (strcmp(code,"410") == 0) puts("You have been eliminated");
    else if (strcmp(code,"411") == 0) puts("You stay in the game");
    else if (strcmp(code,"412") == 0) puts("You become MAIN");
    else if (strcmp(code,"413") == 0) puts("Main answered correctly");
    else if (strcmp(code,"414") == 0) puts("Main answered wrong");
    else if (strcmp(code,"420") == 0) puts("You win the game!");
    else if (strcmp(code,"421") == 0) puts("No winner this round");

    else if (strcmp(code,"299") == 0) puts("You must login before joining a game");
    else puts(code);
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

// MENU
void menu(int logged_in) {
    puts("\n MENU ");
    if (!logged_in) {
        puts("1) REGISTER");
        puts("2) LOGIN");
        puts("3) EXIT");
    } else {
        puts("1) JOIN GAME ROOM");
        puts("2) LOGOUT");
        puts("3) EXIT");
    }
    printf("Select: ");
}

//GAME PLAY
void gamePlay(int sockfd, const char *my_username) {
    char recvBuff[MAXLINE];
    char sendBuff[MAXLINE];
    int n;
    int eliminated = 0;

    while (1) {

        n = recv(sockfd, recvBuff, sizeof(recvBuff) - 1, 0);
        if (n <= 0) {
            puts("Disconnected from server!");
            return;
        }
        recvBuff[n] = 0;

        // TACH NHIEU MESSAGE THEO \n 
        char *saveptr;
        char *line = strtok_r(recvBuff, "\n", &saveptr);

        while (line != NULL) {

            // QUESTION 
            if (strncmp(line, "QUES|", 5) == 0) {

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

                int ans = 0;
                do {
                    printf("Enter your answer (1-4): ");
                    if (scanf("%d", &ans) != 1) {
                        int c;
                        while ((c = getchar()) != '\n' && c != EOF) {}
                        ans = 0;
                    }
                } while (ans < 1 || ans > 4);

                int c;
                while ((c = getchar()) != '\n' && c != EOF) {}

                snprintf(sendBuff, sizeof(sendBuff), "ANSWER %d\n", ans);
                send(sockfd, sendBuff, strlen(sendBuff), 0);

                puts("Answer sent. Waiting for result...");
            }

            //CHON MAIN (STAGE 1)
            else if (strncmp(line, "RESULT|", 7) == 0) {

                char tmp[MAXLINE];
                strncpy(tmp, line, sizeof(tmp));
                tmp[sizeof(tmp) - 1] = 0;

                strtok(tmp, "|");
                char *winner = strtok(NULL, "|");

                if (winner && strcmp(winner, "NONE") == 0) {
                    puts("No winner for this question.");
                }
                else if (winner && strcmp(winner, my_username) == 0) {
                    puts("You are the MAIN player!");
                }
                else {
                    puts("You are a SUB player.");
                }
            }

            // SCORE
            else if (strncmp(line, "SCORE|YOU|", 10) == 0) {
                strtok(line, "|"); // SCORE
                strtok(NULL, "|"); // YOU
                char *score = strtok(NULL, "|");
                printf("Your score: %s\n", score);
            }
            
            else if (strncmp(line, "LOG|", 4) == 0) {
                printf("[SPECTATOR] %s\n", line + 4);
            }

            // MAIN ROUND RESULT (CODE)
            else {
                if (isdigit((unsigned char)line[0])) {
                    explain_code(line);
                    if (strcmp(line, "410") == 0) {
                       eliminated = 1;
                       //return;   // quay ve menu
                       puts("You are eliminated. Watching the game...");
                    }
                }
                // message khac
                else if (strcmp(line, "300") != 0) {
                    printf("[SERVER]: %s\n", line);
                }
            }

            line = strtok_r(NULL, "\n", &saveptr);
        }
    }
}


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

