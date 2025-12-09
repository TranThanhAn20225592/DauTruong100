#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ACCOUNT_FILE "account.txt"

typedef struct {
    char username[50];
    char password[50];
} Account;

Account accounts[1000];
int accountCount = 0;

// LOAD ACCOUNT
int loadAccounts() {
    FILE *f = fopen(ACCOUNT_FILE, "r");
    if (!f) return 0;

    accountCount = 0;
    while (!feof(f)) {
        Account acc;
        if (fscanf(f, "%s %s", acc.username, acc.password) == 2) {
            accounts[accountCount++] = acc;
        }
    }
    fclose(f);
    return accountCount;
}

// SAVE ACCOUNT
void saveAccounts() {
    FILE *f = fopen(ACCOUNT_FILE, "w");
    if (!f) return;

    for (int i = 0; i < accountCount; i++) {
        fprintf(f, "%s %s\n",
                accounts[i].username,
                accounts[i].password);
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
	loadAccounts();
	
    if (findAccount(username) != -1) {
        return 101; 
    }

    strcpy(accounts[accountCount].username, username);
    strcpy(accounts[accountCount].password, password);
    accountCount++;

    saveAccounts();
    return 100; // OK
}

// LOGIN
int loginAccount(const char *username, const char *password) {
    int idx = findAccount(username);
    if (idx == -1) return 112; 

    if (strcmp(accounts[idx].password, password) != 0)
        return 111; // sai password

    return 110; // login thành công
}

// LOGOUT
int logoutAccount(const char *username) {
    int idx = findAccount(username);
    if (idx == -1) return 112; 
    return 120;
}

