#ifndef QUESTION_H
#define QUESTION_H

typedef struct {
    char text[256];
    char options[4][64];
    int correct_answer; //1: A, 2: B, 3: C, 4: D
} Question;

extern Question *questions; // Mảng động hoặc tĩnh tùy ý
extern int questionCount;

int loadQuestions(const char *filename);
int getCorrectAnswer();

#endif
