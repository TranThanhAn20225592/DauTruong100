#include <stdio.h>
#include <string.h>
#include "player.h"

Player players[MAX_PLAYERS];
int playerCount = 0;

void initPlayers() {
    playerCount = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].sockfd = -1;
        players[i].state = 0;
        players[i].answered = 0;
    }
}

int addPlayer(int sockfd, char *username) {
    if (playerCount >= MAX_PLAYERS) return -1;

    players[playerCount].sockfd = sockfd;
    strncpy(players[playerCount].username, username,
            sizeof(players[playerCount].username) - 1);
    players[playerCount].username[
        sizeof(players[playerCount].username) - 1] = '\0';

    players[playerCount].score = 10;
    players[playerCount].state = 1;    // active
    players[playerCount].role = 0;     // sub player

    players[playerCount].currentAnswer = 0;
    players[playerCount].answered = 0;
    players[playerCount].isCorrect = 0;
    players[playerCount].response_time_ms = 0;
    players[playerCount].time_limit_ms = 0;
    players[playerCount].isTimeout = 0;
    players[playerCount].isSkipped = 0;
    players[playerCount].skip_left = 2;

    playerCount++;
    return playerCount - 1;
}

/*
 * KHONG xoa player khoi mang
 * CHI danh dau state = 0
 * Tu dong mark answered de tranh block round
 */
void removePlayer(int sockfd) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].sockfd == sockfd) {
            // Neu disconnect thi xem nhu tra loi sai
            players[i].answered = 1;
            players[i].isCorrect = 0;
            players[i].isTimeout = 1;   // coi disconnect = timeout
            return;
        }
    }
}

/*
 * Reset trang thai cho cau hoi moi
 * CHI reset player con song
 */
void resetPlayerAnswers() {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) {
            players[i].currentAnswer = 0;
            players[i].answered = 0;
            players[i].isCorrect = 0;
            players[i].response_time_ms = 0;
            players[i].isTimeout = 0;
            players[i].isSkipped = 0;
        }
    }
}

Player* getPlayer(int sockfd) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].sockfd == sockfd && players[i].state == 1) {
            return &players[i];
        }
    }
    return NULL;
}

