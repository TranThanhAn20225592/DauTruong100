#include <time.h>
#include "join.h"

#define WAITING_MAX 100     // so nguoi toi da trong phòng cho
#define MIN_START   2       // so nguoi toi thieu de bat dau khi het thoi gian
#define TIME_LIMIT  20      // thoi gian cho (giây)

int waitingRoom[WAITING_MAX];
int waitingCount = 0;
time_t startTime = 0;

// Khoi tao / reset phòng
void initWaitingRoom() {
    waitingCount = 0;
    startTime = 0;

    for (int i = 0; i < WAITING_MAX; i++) {
        waitingRoom[i] = -1;
    }
}

// Kiem tra timeout chi JOIN
// Tra ve:
//   0   = không timeout
//   210 = het thoi gian, du nguoi, bat dau game
//   202 = het thoi gian, chi 1 nguoi, không du nguoi
int checkJoinTimeout() {
    if (waitingCount == 0 || startTime == 0)
        return 0;

    time_t now = time(NULL);

    if (now - startTime < TIME_LIMIT)
        return 0;

    // Het 20s
    if (waitingCount >= MIN_START) {
        //initWaitingRoom();
        return 210;  // bat dau tran
    }

    // Chi có 1 nguoi
    //initWaitingRoom();
    return 202;      // không du nguoi
}

// Xu lý JOIN
// Tra ve:
//   200 = Join thành công
//   201 = Phòng day
//   210 = Phòng du 100 nguoi, bat dau game ngay
int handleJoin(int sockfd) {

    // Phòng dã du 100 nguoi
    if (waitingCount >= WAITING_MAX)
        return 201;

    // Thêm nguoi choi vào danh sách
    waitingRoom[waitingCount++] = sockfd;

    // Neu là nguoi dau tiên, bat dau dem gio
    if (waitingCount == 1)
        startTime = time(NULL);

    // Ðã du 100 nguoi, bat dau tran
    if (waitingCount == WAITING_MAX) {
        initWaitingRoom();
        return 210;
    }

    // Chua du nguoi nhung join OK
    return 200;
}

