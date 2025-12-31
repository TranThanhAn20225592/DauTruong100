#include "menu.h"
#include <stdio.h>

void menu(int logged_in) {
    puts("\n MENU ");
    if (!logged_in) {
        puts("1) REGISTER");
        puts("2) LOGIN");
        puts("3) EXIT");
    } else {
        puts("1) JOIN GAME ROOM");
        puts("2) LOGOUT");
        puts("3) EXIT");
    }
    printf("Select: ");
}
