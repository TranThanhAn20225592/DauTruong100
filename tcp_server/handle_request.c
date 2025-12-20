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

// GLOBAL
char onlineUser[1000][50];
int onlineCount = 0;
extern int gameState;

// 0: chon main, 1: luot choi ch�nh 
int roundPhase = 0;

// dem so nguoi da tra loi trong luot hien tai 
int activeAnswerCount = 0;

// USER ONLINE
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

// MAIN HANDLER

int handleRequest(
    char *buff,
    int idx,
    int client_fd,
    ClientSession *sessions
) {

    char cmd[64] = {0};
    char arg1[50] = {0};
    char arg2[50] = {0};

    int cnt = sscanf(buff, "%s %s %s", cmd, arg1, arg2);
    int code = 199;

    // REGISTER
    if (strcmp(cmd, "REGISTER") == 0) {
        if (cnt == 3)
            code = registerAccount(arg1, arg2);
    }

    // LOGIN
    else if (strcmp(cmd, "LOGIN") == 0) {
        if (cnt == 3) {
            if (isUserOnline(arg1)) {
                code = 113;
            } else {
                code = loginAccount(arg1, arg2);
                if (code == 110) {
                    sessions[idx].isLoggedIn = 1;
                    strcpy(sessions[idx].username, arg1);
                    setUserOnline(arg1);
                }
            }
        }
    }

    // LOGOUT
    else if (strcmp(cmd, "LOGOUT") == 0) {
        if (!sessions[idx].isLoggedIn) {
            code = 121;
        } else {
            code = logoutAccount(sessions[idx].username);
            if (code == 120) {
                setUserOffline(sessions[idx].username);
                sessions[idx].isLoggedIn = 0;
                sessions[idx].username[0] = 0;
            }
        }
    }

    // JOIN
    else if (strcmp(cmd, "JOIN") == 0) {
        if (!sessions[idx].isLoggedIn) return 299;
        if (gameState == 1) return 203;
        return handleJoin(client_fd);
    }

    // ANSWER
    else if (strcmp(cmd, "ANSWER") == 0 && cnt == 2) {

        int ans_val = atoi(arg1);
        Player *p = getPlayer(client_fd);

        if (!p || p->state != 1) return 301;
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

        activeAnswerCount++;

        // �EM SO PLAYER CON SONG
        int aliveCount = 0;
        for (int i = 0; i < playerCount; i++) {
            if (players[i].state == 1)
                aliveCount++;
        }

        // TAT CA �A TRA LOI
        if (activeAnswerCount == aliveCount) {

            // CHON MAIN PLAYER  
            if (roundPhase == 0) {

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

                for (int i = 0; i < playerCount; i++)
                    players[i].role = 0;

                if (winner != -1)
                    players[winner].role = 1;

                broadcastResult(winner);

                if (winner != -1) {
                    roundPhase = 1;
                    startMainRound();
                }
            }

            // MAIN GAME 
            else if (roundPhase == 1) {

                processMainRoundResult();
                broadcastScores();
                currentQuestionId++;

                if (currentQuestionId < questionCount) {
                    sendQuestionToAllPlayers(currentQuestionId);
                } else {
                    printf("[GAME] OUT OF QUESTIONS - END GAME\n");
                    gameState = 0;
                }
            }
        }

        return 300;
    }

    return code;
}

