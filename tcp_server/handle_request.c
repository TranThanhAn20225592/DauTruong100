#include <stdio.h>
#include <string.h>
#include "handle_request.h"
#include "account.h"
#include "join.h"
#include "player.h"

char onlineUser[1000][50];
int onlineCount = 0;

extern int gameState;

int isUserOnline(char *username) {
    for (int i = 0; i < onlineCount; i++) {
        if (strcmp(onlineUser[i], username) == 0)
            return 1;
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

int handleRequest(
    char *buff,
    int i,
    int client_fd,
    int logged_in[],
    char client_user[][50]
) {
    // char cmd[64], u[50], p[50];
    // int answer = 0;
    // cmd[0]=u[0]=p[0]=0;

    char cmd[64] = {0};
    char arg1[50] = {0};
    char arg2[50] = {0};

    int cnt = sscanf(buff, "%s %s %s", cmd, arg1, arg2);
    int code = 199; // default: invalid

    // REGISTER
    if (strcmp(cmd, "REGISTER") == 0) {
        char *u = arg1;
        char *p = arg2;
        if (cnt == 3)
            code = registerAccount(u, p);
    }

    // LOGIN
    else if (strcmp(cmd, "LOGIN") == 0) {
        char *u = arg1;
        char *p = arg2;
        if (cnt == 3) {
            if (isUserOnline(u)) code = 113;
            else {
                code = loginAccount(u, p);
                if (code == 110) {
                    logged_in[i] = 1;
                    strcpy(client_user[i], u);
                    setUserOnline(u);
                }
            }
        }
    }

    // LOGOUT
    else if (strcmp(cmd, "LOGOUT") == 0) {
        if (!logged_in[i]) code = 121;
        else {
            code = logoutAccount(client_user[i]);
            if (code == 120) {
                setUserOffline(client_user[i]);
                logged_in[i] = 0;
                client_user[i][0] = 0;
            }
        }
    }    
    // JOIN
    else if (strcmp(cmd, "JOIN") == 0) {
        if (!logged_in[i]) return 299; // phai dang 
        if (gameState == 1) return 203; // khong the join khi dang choi
        code = handleJoin(client_fd);  // tra: 200,201,210,299
        return code;
    } 
    //ANSWER
    else if (strcmp(cmd, "ANSWER") == 0) {
        char *a = arg1; // Vì cú pháp là "ANSWER <số>" nên arg1 chính là đáp án

        if (cnt == 2) { // Cần ít nhất cmd và arg1
            int ans_val = atoi(a); 
            // Xử lý logic game...
            Player *p = getPlayer(client_fd);
            if (p != NULL && p->state == 1) {
                p->currentAnswer = ans_val;
                printf("[GAME] Player %s answered: %d\n", p->username, ans_val);
                code = 300; // Mã phản hồi cho việc nhận câu trả lời
            } else {
                code = 301; // Mã phản hồi cho người chơi đã bị loại hoặc không tồn tại
            }
        }
    }   
    else {
        code = 199;
    }

    return code;
}

