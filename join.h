#ifndef JOIN_H
#define JOIN_H

#include <time.h>

extern int waitingRoom[];
extern int waitingCount;
extern time_t startTime;

void initWaitingRoom();
int handleJoin(int sockfd);
int checkJoinTimeout();

#endif

