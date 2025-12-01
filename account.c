#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define ACCOUNT_FILE "account.txt"

typedef struct {
    char username[50];
    char password[50];
    int isLoggedIn;   // 0 = not login, 1 = login
} Account;

Account accounts[1000];
int accountCount = 0;

int loadAccounts() {
    FILE *f = fopen(ACCOUNT_FILE, "r");
    if (!f) {
        return 0;
    }

    accountCount = 0;
    while (!feof(f)) {
        Account acc;
        if (fscanf(f, "%s %s %d", acc.username, acc.password, &acc.isLoggedIn) == 3) {
            accounts[accountCount++] = acc;
        }
    }
    fclose(f);
    return accountCount;
}

void saveAccounts() {
    FILE *f = fopen(ACCOUNT_FILE, "w");
    if (!f) return;

    for (int i = 0; i < accountCount; i++) {
        fprintf(f, "%s %s %d\n",
                accounts[i].username,
                accounts[i].password,
                accounts[i].isLoggedIn);
    }
    fclose(f);
}

// FIND ACCOUNT 
int findAccount(const char *username) {
    for (int i = 0; i < accountCount; i++) {
        if (strcmp(accounts[i].username, username) == 0)
            return i;
    }
    return -1;
}

// REGISTER 
int registerAccount(const char *username, const char *password) {
    if (findAccount(username) != -1) {
        return 101; // Account ton tai
    }

    strcpy(accounts[accountCount].username, username);
    strcpy(accounts[accountCount].password, password);
    accounts[accountCount].isLoggedIn = 0;
    accountCount++;

    saveAccounts();

    return 100; // OK
}

// LOGIN 
int loginAccount(const char *username, const char *password) {
    int idx = findAccount(username);
    if (idx == -1) return 112; // account không ton tai

    if (accounts[idx].isLoggedIn == 1)
        return 113; // Ðang login nõi khác

    if (strcmp(accounts[idx].password, password) != 0)
        return 111; // sai pass

    accounts[idx].isLoggedIn = 1;
    saveAccounts();    // Ghi lai trang thái login

    return 110; // login thành công
}

// LOGOUT
int logoutAccount(const char *username) {
    int idx = findAccount(username);
    if (idx == -1) return 112; // không ton tai

    if (accounts[idx].isLoggedIn == 0)
        return 121; // chua login

    accounts[idx].isLoggedIn = 0;
    saveAccounts();

    return 120; // logout thành công
}

