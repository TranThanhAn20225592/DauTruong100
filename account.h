#ifndef ACCOUNT_H
#define ACCOUNT_H

#define ACCOUNT_FILE "account.txt"
#define MAX_ACCOUNTS 1000
#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50

// CODE 
#define REGISTER_OK       100
#define REGISTER_EXIST    101

#define LOGIN_OK          110
#define LOGIN_WRONG_PASS  111
#define LOGIN_NO_EXIST    112
#define LOGIN_ALREADY     113

#define LOGOUT_OK         120
#define LOGOUT_NOT_LOGIN  121

#define COMMAND_ERROR     199

// ===========================
// Struct Account
// ===========================
typedef struct {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    int isLoggedIn;   // 0 = chýa login, 1 = ðang login
} Account;


extern Account accounts[MAX_ACCOUNTS];
extern int accountCount;


int loadAccounts();                           // Load accounts to file
void saveAccounts();                           // Save accounts to file
int findAccount(const char *username);        // Find account
int registerAccount(const char *username, const char *password); // Register 
int loginAccount(const char *username, const char *password);    // Login 
int logoutAccount(const char *username);                             // Logout

#endif

