#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "game.h"
#include "player.h"
#include "question.h"

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

    snprintf(buffer, sizeof(buffer), "QUES|%s|%s|%s|%s|%s\n",
             q->text,
             q->options[0],
             q->options[1],
             q->options[2],
             q->options[3]);

    printf("Sending question %d to all players.\n", questionId);

    for (int i = 0; i < playerCount; i++) {
        if (players[i].state == 1) { // Chỉ gửi cho người chơi còn sống
            send(players[i].sockfd, buffer, strlen(buffer), 0);
        }
    }
}