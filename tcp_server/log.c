#include <stdio.h>
#include <time.h>
#include "log.h"

void writeLog(
    const char *function, 
    const char *value, 
    const char *result) {
    FILE *f = fopen("log.txt", "a");
    if (!f) return;

    time_t now = time(NULL);
    time_t nowVN = now + 7 * 3600;
    struct tm *t = localtime(&nowVN);
    if (!t) {
        fclose(f);
        return;
    }

    fprintf(f, "[%02d/%02d/%04d %02d:%02d:%02d] $ %s $ %s $ %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900, t->tm_hour, t->tm_min, t->tm_sec, function, value ? value : "", result ? result : "");
    fclose(f);
}