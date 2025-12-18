#ifndef GAME_H
#define GAME_H

#include <sys/time.h>

/* ====== BIẾN TOÀN CỤC ====== */
extern struct timeval question_start_time;
extern int currentQuestionId;

/* ====== STAGE 1 – GỬI CÂU HỎI / CHỌN MAIN ====== */
void sendQuestionToAllPlayers(int questionId);
void broadcastResult(int winner_idx);

/* ====== STAGE 2 – LƯỢT CHƠI CHÍNH ====== */
int  getMainPlayerIndex(void);
void startMainRound(void);
void processMainRoundResult(void);
void handleMainCorrect(void);
void handleMainWrong(void);
void broadcastScores(void);

#endif
