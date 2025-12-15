#include <time.h>
#include "join.h"

#define WAITING_MAX 100     // so nguoi toi da trong phong cho
#define MIN_START   2       // so nguoi toi thieu de bat dau khi het thoi gian
#define TIME_LIMIT  20      // thoi gian cho (giay)

int waitingRoom[WAITING_MAX];
int waitingCount = 0;
time_t startTime = 0;

// Khoi tao / reset phong
void initWaitingRoom() {
    waitingCount = 0;
    startTime = 0;

    for (int i = 0; i < WAITING_MAX; i++) {
        waitingRoom[i] = -1;
    }
}

// Kiem tra timeout khi JOIN
int checkJoinTimeout() {
    if (waitingCount == 0 || startTime == 0)
        return 0;

    time_t now = time(NULL);

    if (now - startTime < TIME_LIMIT)
        return 0;

    if (waitingCount >= MIN_START) {
        return 210; 
    }

    return 202;
}

// Xu ly JOIN
int handleJoin(int sockfd) {

    if (waitingCount >= WAITING_MAX)
        return 201;

    waitingRoom[waitingCount++] = sockfd;

    if (waitingCount == 1)
        startTime = time(NULL);

    if (waitingCount == WAITING_MAX) {
        return 210;
    }

    return 200;
}

