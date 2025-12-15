#ifndef PLAYER_H
#define PLAYER_H

#include <netinet/in.h>

#define MAX_PLAYERS 100

typedef struct {
    int sockfd;
    char username[50];
    int score;
    int state; //0: bi loai; 1: con song
    int role; //0: phu; 1: chinh

    int currentAnswer;
    int answered;              // 0: chua tra loi, 1: da tra loi
    int isCorrect;             // 0: sai, 1: dung
    long response_time_ms;     // thoi gian tra loi (ms)

} Player;

extern Player players[MAX_PLAYERS];
extern int playerCount;

void initPlayers();
int addPlayer(int sockfd, char *username);
void removePlayer(int sockfd);
void resetPlayerAnswers();
Player* getPlayer(int sockfd);

#endif
