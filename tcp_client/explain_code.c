#include "explain_code.h"
#include <stdio.h>
#include <string.h>

void explain_code(const char *code) {
    if (strcmp(code,"900") == 0) puts("Connected to server");
    else if (strcmp(code,"100") == 0) puts("Register successful");
    else if (strcmp(code,"101") == 0) puts("Account already exists");
    else if (strcmp(code,"110") == 0) puts("Login successful");
    else if (strcmp(code,"111") == 0) puts("Wrong password");
    else if (strcmp(code,"112") == 0) puts("Account does not exist");
    else if (strcmp(code,"113") == 0) puts("Account already logged in");
    else if (strcmp(code,"120") == 0) puts("Logout successful");
    else if (strcmp(code,"121") == 0) puts("Logout failed");

    else if (strcmp(code,"200") == 0) puts("JOIN OK - Waiting for other players...");
    else if (strcmp(code,"201") == 0) puts("Player limit exceeded!");
    else if (strcmp(code,"202") == 0) puts("Insufficient number of player!");
    else if (strcmp(code,"203") == 0) puts("Cannot JOIN during an ongoing game!");
    else if (strcmp(code,"210") == 0) puts("GAME START!");

    else if (strcmp(code,"300") == 0) puts("Answer received");
    else if (strcmp(code,"301") == 0) puts("You are not in the game");
    else if (strcmp(code,"302") == 0) puts("You have already answered");
    else if (strcmp(code,"308") == 0) puts("Skip completed but no scores changed");
    
    else if (strcmp(code,"400") == 0) puts("Answer correct");
    else if (strcmp(code,"401") == 0) puts("Answer wrong");
    else if (strcmp(code,"402") == 0) puts("Answer timeout");
    else if (strcmp(code,"410") == 0) puts("You have been eliminated");
    else if (strcmp(code,"411") == 0) puts("You stay in the game");
    else if (strcmp(code,"412") == 0) puts("You become MAIN");
    else if (strcmp(code,"413") == 0) puts("Main answered correctly");
    else if (strcmp(code,"414") == 0) puts("Main answered wrong");
    else if (strcmp(code,"420") == 0) puts("You win the game!");
    else if (strcmp(code,"421") == 0) puts("No winner this round");
    else if (strcmp(code,"422") == 0) puts("Main player skipped the question, next question");
    else if (strcmp(code,"423") == 0) puts("You skipped this question");
    else if (strcmp(code,"424") == 0) puts("Only MAIN player can skip");

    else if (strcmp(code,"299") == 0) puts("You must login before joining a game");
    else puts(code);
}

