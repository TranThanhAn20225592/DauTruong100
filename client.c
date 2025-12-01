#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 4096

// CODE Server 
void explain_code(const char *code) {
    if (strcmp(code,"100") == 0) puts("Register successful");             
    else if (strcmp(code,"101") == 0) puts("Account already exists");     
    else if (strcmp(code,"110") == 0) puts("Login successful");               
    else if (strcmp(code,"111") == 0) puts("Wrong password");                  
    else if (strcmp(code,"112") == 0) puts("Account does not exist");          
    else if (strcmp(code,"113") == 0) puts("This account is already logged in elsewhere"); 
    else if (strcmp(code,"120") == 0) puts("Logout successful");
    else if (strcmp(code,"121") == 0) puts("Logout failed, you are not logged in");
    else if (strcmp(code,"199") == 0) puts("Error / Invalid command");
    else printf("Server returned code: %s\n", code);
}

// menu
void menu(void) {
    puts("\n MENU ");
    puts("1) REGISTER");
    puts("2) LOGIN");
    puts("3) LOGOUT");
    puts("4) EXIT");
    printf("Select [1-4]: ");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr,"Usage: %s <ServerIP> <Port>\n", argv[0]);
        return 1;
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);

    int sockfd;
    struct sockaddr_in servaddr;
    char sendBuff[MAXLINE], recvBuff[MAXLINE];

    // create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &servaddr.sin_addr)<=0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    if (connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0) {
        perror("connect");
        close(sockfd);
        return 1;
    }

    // greeting code
    ssize_t n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
    if (n>0) {
        recvBuff[n]=0;
        explain_code(recvBuff);
    }

    int choice;
    while (1) {
        menu();
        if (scanf("%d", &choice)!=1) break;
        while (getchar()!='\n'); // clear stdin

        if (choice == 1) { // REGISTER
            char username[50], password[50];
            printf("Username: "); scanf("%49s", username); while(getchar()!='\n');
            printf("Password: "); scanf("%49s", password); while(getchar()!='\n');

            snprintf(sendBuff,sizeof(sendBuff),"REGISTER %s %s", username,password);
            send(sockfd, sendBuff, strlen(sendBuff),0);

            n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
            if (n<=0) { puts("Server is not connect."); break; }
            recvBuff[n]=0;
            explain_code(recvBuff);
        }
        else if (choice == 2) { // LOGIN
            char username[50], password[50];
            printf("Username: "); scanf("%49s", username); while(getchar()!='\n');
            printf("Password: "); scanf("%49s", password); while(getchar()!='\n');

            snprintf(sendBuff,sizeof(sendBuff),"LOGIN %s %s", username,password);
            send(sockfd, sendBuff, strlen(sendBuff),0);

            n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
            if (n<=0) { puts("Server is not connect."); break; }
            recvBuff[n]=0;
            explain_code(recvBuff);
        }
        else if (choice == 3) { // LOGOUT
            snprintf(sendBuff,sizeof(sendBuff),"LOGOUT");
            send(sockfd, sendBuff, strlen(sendBuff),0);

            n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
            if (n<=0) { puts("Server is not connect."); break; }
            recvBuff[n]=0;
            explain_code(recvBuff);
        }
        else if (choice == 4) { // EXIT
            puts("Exit client.");
            break;
        }
        else {
            puts("Select not found");
        }
    }

    close(sockfd);
    return 0;
}

