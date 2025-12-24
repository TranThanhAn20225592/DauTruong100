#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <limits.h>

#include "game.h"
#include "player.h"
#include "question.h"


// GLOBAL

struct timeval question_start_time;
int currentQuestionId = 0;

void sendCode(Player *p, int code) {
    char buf[16];
    snprintf(buf, sizeof(buf), "%d\n", code);
    send(p->sockfd, buf, strlen(buf), 0);
}

// UTIL

int getMainPlayerIndex() {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1 && players[i].role == 1)
            return i;
    }
    return -1;
}

// TIME RULES

void setTimeLimits() {

    int mainIdx = getMainPlayerIndex();

    for (int i = 0; i < playerCount; i++) {

        if (players[i].state != 1)
            continue;

        if (i == mainIdx)
            players[i].time_limit_ms = 60000; // MAIN 60s
        else
            players[i].time_limit_ms = 30000; // SUB 30s
    }
}

void applyTimeoutRules() {

    for (int i = 0; i < playerCount; i++) {

        if (players[i].state != 1)
            continue;

        if (players[i].response_time_ms > players[i].time_limit_ms) {
            players[i].isCorrect = 0;
            players[i].isTimeout = 1;
        } else {
        	players[i].isTimeout = 0;
		} 
    }
}


// SEND QUESTION � d�ng cho ca 2 stage 
void sendQuestionToAllPlayers(int questionId) {

    if (questionId >= questionCount) {
        printf("Out of questions!\n");
        return;
    }

    // RESET COUNTER TRA LOI 
    extern int activeAnswerCount;
    activeAnswerCount = 0;

    currentQuestionId = questionId;
    Question *q = &questions[questionId];
    char buffer[1024];

    // reset trang th�i tra loi cua player
    resetPlayerAnswers();
    gettimeofday(&question_start_time, NULL);

    snprintf(buffer, sizeof(buffer),
        "QUES|%s|%s|%s|%s|%s\n",
        q->text,
        q->options[0],
        q->options[1],
        q->options[2],
        q->options[3]);

    printf("[GAME] Send question %d\n", questionId);

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) {
            if (players[i].role == 1) {
                char skipMsg[64];
                snprintf(skipMsg, sizeof(skipMsg), "SKIP_INFO|%d\n", players[i].skip_left);
                send(players[i].sockfd, skipMsg, strlen(skipMsg), 0);
            }
            send(players[i].sockfd, buffer, strlen(buffer), 0);
        }
    }
}



// RESULT FOR SELECT MAIN PLAYER  
void broadcastResult(int winner_idx) {

    char msg[256];

    if (winner_idx == -1) {
        strcpy(msg, "RESULT|NONE\n");
    } else {
        snprintf(msg, sizeof(msg),
            "RESULT|%s|%ld\n",
            players[winner_idx].username,
            players[winner_idx].response_time_ms);
    }

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) {
            send(players[i].sockfd, msg, strlen(msg), 0);
        }
    }
}

//  STAGE 2 
// MAIN ROUND 
void startMainRound() {
    printf("[GAME] MAIN ROUND START\n");
    setTimeLimits();
    sendQuestionToAllPlayers(currentQuestionId);
}

// MAIN PLAYER CORRECT
void handleMainCorrect() {

    int mainIdx = getMainPlayerIndex();
    if (mainIdx == -1) return;

    Player *main = &players[mainIdx];
    int stolenScore = 0;
    int subCorrect = 0;

    sendCode(main, 413); // MAIN CORRECT

    for (int i = 0; i < playerCount; i++) {

        if (players[i].state != 1 || i == mainIdx)
            continue;

        if (!players[i].isCorrect) {

            stolenScore += players[i].score;
            players[i].score = 0;
            players[i].state = 0;

            if (players[i].isTimeout) {
                sendCode(&players[i], 402); // TIMEOUT
            } else {
                sendCode(&players[i], 401); // WRONG
            }
            sendCode(&players[i], 410); // ELIMINATED

            // GUI LOG SPECTATOR
            char log[128];
            snprintf(log, sizeof(log),
                "LOG|%s was eliminated\n",
                players[i].username);
            sendLogToSpectators(log);

        }

        else {
            subCorrect++;
            sendCode(&players[i], 400); // SUB CORRECT
            sendCode(&players[i], 411); // STAY IN GAME
        }
    }

    
    main->score += stolenScore;

    if (subCorrect == 0) {

        sendCode(main, 420); // MAIN WINS GAME

        char log[128];
        snprintf(log, sizeof(log),
            "LOG|MAIN %s wins the game\n",
            main->username);
        sendLogToSpectators(log);

        printf("[GAME] MAIN %s WINS GAME\n", main->username);
        for (int i = 0; i < playerCount; i++) {
            if (players[i].state == 0 && players[i].sockfd > 0) {
                sendCode(&players[i], 421); 
            }
        }
    }
}


// MAIN PLAYER WRONG 
void handleMainWrong() {

    int mainIdx = getMainPlayerIndex();
    if (mainIdx == -1) return;

    Player *main = &players[mainIdx];
    int aliveSub = 0;
    int nextMain = -1;
    long bestTime = LONG_MAX;

    int mainScore = main->score;

    sendCode(main, 414); // MAIN WRONG
    sendCode(main, 410); // ELIMINATED

    main->score = 0;
    main->state = 0;

    char log[128];
    snprintf(log, sizeof(log),
        "LOG|MAIN %s was eliminated\n",
        main->username);
    sendLogToSpectators(log);

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1 && players[i].role == 0)
            aliveSub++;
    }

    if (aliveSub == 0) return;

    int share = mainScore / aliveSub;

    for (int i = 0; i < playerCount; i++) {

        if (players[i].state != 1 || players[i].role == 1)
            continue;

        players[i].score += share;

        if (players[i].isCorrect &&
            players[i].response_time_ms < bestTime) {

            bestTime = players[i].response_time_ms;
            nextMain = i;
        }
    }

    for (int i = 0; i < playerCount; i++)
        players[i].role = 0;

    if (nextMain != -1) {

        players[nextMain].role = 1;
        sendCode(&players[nextMain], 412); // BECOME MAIN

        char log2[128];
        snprintf(log2, sizeof(log2),
            "LOG|%s becomes MAIN\n",
            players[nextMain].username);
        sendLogToSpectators(log2);

    }

    else {
        for (int i = 0; i < playerCount; i++) {
            if (players[i].state == 1)
                sendCode(&players[i], 421); // NO WINNER
        }
        sendLogToSpectators("LOG|No winner this round\n");
    }
}

void handleMainSkip() {

    int mainIdx = getMainPlayerIndex();
    if (mainIdx == -1) return;

    Player *main = &players[mainIdx];

    int totalSubWrongScore = 0;
    int subCorrectCount = 0;

    // diem SUB ��ng v� gom diem SUB sai
    for (int i = 0; i < playerCount; i++) {

        if (players[i].state != 1 || i == mainIdx)
            continue;

        if (players[i].isCorrect) {
            subCorrectCount++;
        } else {
            totalSubWrongScore += players[i].score;
        }
    }

    // neu kh�ng c� SUB d�ng kh�ng ai duoc diem 
	    if (subCorrectCount == 0) {
        sendCode(main, 308); // NO WINNER
        return;
    }

    // MAIN chia 1/2 diem
    int halfMainScore = main->score / 2;
    main->score -= halfMainScore;

    int share =
        (halfMainScore + totalSubWrongScore) / subCorrectCount;

    // chia cho SUB d�ng
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1 &&
            players[i].role == 0 &&
            players[i].isCorrect) {

            players[i].score += share;
            sendCode(&players[i], 400); // SUB CORRECT
        }
    }

    sendCode(main, 430); // MAIN SKIPPED
}

// Cap nhat score sau moi cau hoi trong luot choi chinh  
void broadcastScores() {
    char buf[64];

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state != 1) continue;

        snprintf(buf, sizeof(buf),
            "SCORE|YOU|%d\n",
            players[i].score);

        send(players[i].sockfd, buf, strlen(buf), 0);
    }
}

void sendLogToSpectators(const char *msg) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 0) {   // cho spectator
            send(players[i].sockfd, msg, strlen(msg), 0);
        }
    }
}

// MAIN ROUND RESULT
void processMainRoundResult() {

    int mainIdx = getMainPlayerIndex();
    if (mainIdx == -1) return;
    
    applyTimeoutRules();

    if (players[mainIdx].isSkipped) {
        handleMainSkip();
    } else if (players[mainIdx].isCorrect) {
        handleMainCorrect();
    } else {
        handleMainWrong();
    }

    // KIEM TRA GAME OVER
    int aliveCount = 0;
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1)
            aliveCount++;
    }

    if (aliveCount <= 1) {
        printf("[GAME] GAME OVER\n");
        // Not send question 
        return;
    }
}


