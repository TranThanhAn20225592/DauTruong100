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

// 0: chon main, 1: luot choi chinh
int roundPhase = 0;

/* ================== UTILS ================== */

static int countAlivePlayers(void) {
    int alive = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) alive++;
    }
    return alive;
}

static int countAnsweredAlivePlayers(void) {
    int ans = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1 && players[i].answered) ans++;
    }
    return ans;
}

/*
 * Gom toàn b? logic "khi t?t c? ngý?i chõi c?n s?ng ð? tr? l?i"
 * ð? dùng chung cho ANSWER / SKIP / DISCONNECT
 */
static void tryAdvanceRound(void) {
    int aliveCount = countAlivePlayers();
    int answeredCount = countAnsweredAlivePlayers();

    if (aliveCount <= 0) return;
    if (answeredCount != aliveCount) return;

    if (roundPhase == 0) {
        // CHON MAIN PLAYER (nguoi dung va nhanh nhat)
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

        for (int i = 0; i < playerCount; i++) {
            players[i].role = 0;
        }
        if (winner != -1) {
            players[winner].role = 1;
        }

        broadcastResult(winner);

        currentQuestionId++;
        if (currentQuestionId >= questionCount) {
            currentQuestionId = 0;
            printf("[GAME] OUT OF QUESTIONS - RETURN TO THE FIRST QUESTION OF THE LIST\n");
        }

        if (winner != -1) {
            roundPhase = 1;
            startMainRound();
        } else {
            // khong co ai dung -> sang cau tiep theo, van dang phase 0
            sendQuestionToAllPlayers(currentQuestionId);
        }
    } else {
        // MAIN GAME
        processMainRoundResult();
        broadcastScores();

        currentQuestionId++;
        if (currentQuestionId >= questionCount) {
            currentQuestionId = 0;
        }
        sendQuestionToAllPlayers(currentQuestionId);
    }
}

/* ============== USER ONLINE ================= */

int isUserOnline(char *username) {
    for (int i = 0; i < onlineCount; i++) {
        if (strcmp(onlineUser[i], username) == 0) return 1;
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

/* ================= MAIN HANDLER ================= */

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
        return code;
    }

    // LOGIN
    if (strcmp(cmd, "LOGIN") == 0) {
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
        return code;
    }

    // LOGOUT
    if (strcmp(cmd, "LOGOUT") == 0) {
        if (!sessions[idx].isLoggedIn) {
            return 121;
        }
        code = logoutAccount(sessions[idx].username);
        if (code == 120) {
            setUserOffline(sessions[idx].username);
            sessions[idx].isLoggedIn = 0;
            sessions[idx].username[0] = 0;
        }
        return code;
    }

    // JOIN
    if (strcmp(cmd, "JOIN") == 0) {
        if (!sessions[idx].isLoggedIn) return 299;
        if (gameState == 1) return 203;
        return handleJoin(client_fd);
    }

    // ANSWER
    if (strcmp(cmd, "ANSWER") == 0 && cnt == 2) {
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

        // N?u ð? ngý?i tr? l?i -> qua round
        if (gameState == 1) {
            tryAdvanceRound();
        }

        return 300;
    }

    // SKIP
    if (strcmp(cmd, "SKIP") == 0) {
        Player *p = getPlayer(client_fd);
        if (!p || p->state != 1) return 301;

        // chi MAIN duoc skip
        if (p->role != 1) return 305;

        // het luot skip -> eliminated
        if (p->skip_left <= 0) {
            printf("[GAME] Player %s tried to skip with 0 skips left -> Eliminated\n", p->username);

            handleMainWrong();
            broadcastScores();

            currentQuestionId++;
            if (currentQuestionId >= questionCount) currentQuestionId = 0;

            if (currentQuestionId < questionCount && gameState == 1) {
                sendQuestionToAllPlayers(currentQuestionId);
            }
            return 300;
        }

        p->skip_left--;
        p->answered = 1;
        p->isSkipped = 1;

        if (gameState == 1) {
            tryAdvanceRound();
        }

        return 307;
    }

    return code;
}

/* ============ DISCONNECT HANDLER ============ */
/*
 * G?I HÀM NÀY ? server.c KHI recv() <= 0
 */
void handleClientDisconnect(
    int client_fd,
    int idx,
    ClientSession *sessions
) {
    // user offline n?u ðang login
    if (sessions[idx].isLoggedIn) {
        setUserOffline(sessions[idx].username);
        sessions[idx].isLoggedIn = 0;
        sessions[idx].username[0] = 0;
    }

    // mark player disconnected (player.c m?i s? state=0, answered=1 n?u chýa)
    removePlayer(client_fd);

    // n?u ðang ? trong game, th? ð?y round ti?p
    if (gameState == 1) {
        tryAdvanceRound();
    }
}


