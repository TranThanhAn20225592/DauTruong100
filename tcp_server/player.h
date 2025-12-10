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
} Player;

extern Player players[MAX_PLAYERS];
extern int playerCount;

void initPlayers();
int addPlayer(int sockfd, char *username);
void removePlayer(int sockfd);
void resetPlayerAnswers();
Player* getPlayer(int sockfd);

#endif