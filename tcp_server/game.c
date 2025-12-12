#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "game.h"
#include "player.h"
#include "question.h"
#include <sys/time.h>

struct timeval question_start_time;

int currentQuestionId = 0;

void sendQuestionToAllPlayers(int questionId) {
    if (questionId >= questionCount) {
        printf("Out of question!\n");
        return;
    }

    currentQuestionId = questionId;
    Question *q = &questions[questionId];
    char buffer[1024];

    resetPlayerAnswers();
    gettimeofday(&question_start_time, NULL);

    snprintf(buffer, sizeof(buffer), "QUES|%s|%s|%s|%s|%s\n",
             q->text,
             q->options[0],
             q->options[1],
             q->options[2],
             q->options[3]);

    printf("Sending question %d to all players.\n", questionId);

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) { // Chi gui cho nguoi choi con song
            send(players[i].sockfd, buffer, strlen(buffer), 0);
        }
    }
}

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

