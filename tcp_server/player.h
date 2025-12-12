#ifndef PLAYER_H
#define PLAYER_H

#include <netinet/in.h>

#define MAX_PLAYERS 100

typedef struct {
    int sockfd;
    char username[50];
    int score;
    int state; //0: eliminated; 1: alive
    int role; //0: sub; 1: main

    int currentAnswer;
    int answered;              // 0: chưa trả lời, 1: đã trả lời
    int isCorrect;             // 0: sai, 1: đúng
    long response_time_ms;     // thời gian trả lời (ms)

} Player;

extern Player players[MAX_PLAYERS];
extern int playerCount;

void initPlayers();
int addPlayer(int sockfd, char *username);
void removePlayer(int sockfd);
void resetPlayerAnswers();
Player* getPlayer(int sockfd);

#endif
