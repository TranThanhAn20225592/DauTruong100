#ifndef GAME_H
#define GAME_H

#include <sys/time.h>

extern int currentQuestionId;
extern struct timeval question_start_time;

void sendQuestionToAllPlayers(int questionId);
void broadcastResult(int winner_idx);

#endif
