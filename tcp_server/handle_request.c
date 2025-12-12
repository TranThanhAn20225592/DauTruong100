#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>
#include <stdlib.h> 
#include "handle_request.h"
#include "account.h"
#include "join.h"
#include "player.h"
#include "game.h"
#include "question.h" 


char onlineUser[1000][50];
int onlineCount = 0;

extern int gameState;

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

int handleRequest(
    char *buff,
    int i,
    int client_fd,
    int logged_in[],
    char client_user[][50]
) {

    char cmd[64] = {0};
    char arg1[50] = {0};
    char arg2[50] = {0};

    int cnt = sscanf(buff, "%s %s %s", cmd, arg1, arg2);
    int code = 199; // default: invalid

    // REGISTER
    if (strcmp(cmd, "REGISTER") == 0) {
        char *u = arg1;
        char *p = arg2;
        if (cnt == 3)
            code = registerAccount(u, p);
    }

    // LOGIN
    else if (strcmp(cmd, "LOGIN") == 0) {
        char *u = arg1;
        char *p = arg2;
        if (cnt == 3) {
            if (isUserOnline(u)) code = 113;
            else {
                code = loginAccount(u, p);
                if (code == 110) {
                    logged_in[i] = 1;
                    strcpy(client_user[i], u);
                    setUserOnline(u);
                }
            }
        }
    }

    // LOGOUT
    else if (strcmp(cmd, "LOGOUT") == 0) {
        if (!logged_in[i]) code = 121;
        else {
            code = logoutAccount(client_user[i]);
            if (code == 120) {
                setUserOffline(client_user[i]);
                logged_in[i] = 0;
                client_user[i][0] = 0;
            }
        }
    }    
    // JOIN
    else if (strcmp(cmd, "JOIN") == 0) {
        if (!logged_in[i]) return 299; // phai dang 
        if (gameState == 1) return 203; // khong the join khi dang choi
        code = handleJoin(client_fd);  // tra: 200,201,210,299
        return code;
    } 
    
    //ANSWER
    else if (strcmp(cmd, "ANSWER") == 0) {

        if (cnt == 2) {
           int ans_val = atoi(arg1);
           Player *p = getPlayer(client_fd);

           if (p == NULL || p->state != 1) return 301;
           if (p->answered) return 302;

           struct timeval now;
           gettimeofday(&now, NULL);

            long elapsed =
               (now.tv_sec - question_start_time.tv_sec) * 1000 +
               (now.tv_usec - question_start_time.tv_usec) / 1000;

            p->currentAnswer = ans_val;
            p->answered = 1;
            p->response_time_ms = elapsed;
            p->isCorrect = (ans_val == getCorrectAnswer());

            int allAnswered = 1;
            for (int i = 0; i < playerCount; i++) {
                if (players[i].state == 1 && !players[i].answered) {
                   allAnswered = 0;
                   break;
                }
            }

            if (allAnswered) {
                int winner = -1;
                long best_time = LONG_MAX;

                for (int i = 0; i < playerCount; i++) {
                    if (players[i].state == 1 &&
                        players[i].isCorrect &&
                        players[i].response_time_ms < best_time) {

                        best_time = players[i].response_time_ms;
                        winner = i;
                    }
                }

                // set role
                for (int i = 0; i < playerCount; i++)
                players[i].role = 0;

                if (winner != -1)
                players[winner].role = 1;

                broadcastResult(winner);
            }

            return 300;
        }
    }

    else {
        code = 199;
    }

    return code;
}

