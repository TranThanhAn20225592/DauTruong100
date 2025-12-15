#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "question.h"

#define MAX_QUESTIONS 100

Question list[MAX_QUESTIONS];
Question *questions = list;
int questionCount = 0;

int loadQuestions(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Failed to open question file");
        return 0;
    }

    questionCount = 0;
    char buffer[512];

    while (questionCount < MAX_QUESTIONS) {
        if (!fgets(list[questionCount].text, sizeof(list[questionCount].text), f)) break;

        list[questionCount].text[strcspn(list[questionCount].text, "\n")] = 0;
        for (int i = 0; i < 4; i++) {
            if (!fgets(list[questionCount].options[i], sizeof(list[questionCount].options[i]), f)) break;

            list[questionCount].options[i][strcspn(list[questionCount].options[i], "\n")] = 0;
        }

        if (!fgets(buffer, sizeof(buffer), f)) break;

        list[questionCount].correct_answer = atoi(buffer);
        questionCount++;
    }

    fclose(f);
    printf("Loaded %d questions.\n", questionCount);
    return questionCount;
}

int getCorrectAnswer() {
    extern int currentQuestionId;
    return questions[currentQuestionId].correct_answer; 
}
