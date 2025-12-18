#include <stdio.h>
#include <string.h>
#include "player.h"

Player players[MAX_PLAYERS];
int playerCount = 0;

void initPlayers() {
    playerCount = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        players[i].sockfd = -1;
    }
}

int addPlayer(int sockfd, char *username) {
    if (playerCount >= MAX_PLAYERS) return -1;

    players[playerCount].sockfd = sockfd;
    strcpy(players[playerCount].username, username);
    players[playerCount].score = 10;
    players[playerCount].state = 1;
    players[playerCount].role = 0;
    players[playerCount].currentAnswer = 0;
    playerCount++;
    return playerCount - 1;
}

void removePlayer(int sockfd) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].sockfd == sockfd) {
            for (int j = i; j < playerCount - 1; j++) {
                players[j] = players[j + 1];
            }
            playerCount--;
            return;
        }
    }
}

void resetPlayerAnswers() {
    for (int i = 0; i < playerCount; i++) {
        players[i].currentAnswer = 0;
        players[i].answered = 0;
        players[i].isCorrect = 0;
        players[i].response_time_ms = 0;
    }
}

Player* getPlayer(int sockfd) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].sockfd == sockfd) {
            return &players[i];
        }
    }
    return NULL;
}
