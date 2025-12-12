#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE 4096

// CODE Server 
void explain_code(const char *code) {
    if (strcmp(code,"900") == 0) puts("Connected to server");     
    else if (strcmp(code,"100") == 0) puts("Register successful");             
    else if (strcmp(code,"101") == 0) puts("Account already exists");     
    else if (strcmp(code,"110") == 0) puts("Login successful");               
    else if (strcmp(code,"111") == 0) puts("Wrong password");                  
    else if (strcmp(code,"112") == 0) puts("Account does not exist");          
    else if (strcmp(code,"113") == 0) puts("This account is already logged in elsewhere"); 
    else if (strcmp(code,"120") == 0) puts("Logout successful");
    else if (strcmp(code,"121") == 0) puts("Logout failed, you are not logged in");
    else if (strcmp(code,"199") == 0) puts("Error / Invalid command");

    // JOIN ROOM
    else if (strcmp(code,"200") == 0) puts("JOIN OK - Dang cho nguoi choi khac...");
    else if (strcmp(code,"201") == 0) puts("Phong cho da day!");
    else if (strcmp(code,"202") == 0) puts("Khong du nguoi de bat dau tro choi!");
    else if (strcmp(code,"210") == 0) puts("Tro choi bat dau!");
    else if (strcmp(code,"299") == 0) puts("JOIN failed - Ban chua dang nhap!");

    else printf("Server returned code: %s\n", code);
}

// menu cho 2 trang th�i: d� login / chua login
void menu(int logged_in) {
    puts("\n MENU ");
    if (!logged_in) {
        puts("1) REGISTER");
        puts("2) LOGIN");
        puts("3) EXIT");
        printf("Select [1-3]: ");
    } else {
        puts("1) JOIN GAME ROOM");
        puts("2) LOGOUT");
        puts("3) EXIT");
        printf("Select [1-3]: ");
    }
}

void gamePlay(int sockfd) {
    char recvBuff[MAXLINE];
    char sendBuff[MAXLINE];
    int n;

    while (1) {
        printf("\n--- WAITING QUESTION FROM SERVER ---\n");
        
        // 1. Chờ nhận câu hỏi (Blocking)
        n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
        if (n <= 0) {
            puts("Unable to connect to server!");
            return;
        }
        recvBuff[n] = 0;

        // 2. Kiểm tra xem có phải gói tin QUES không?
        if (strncmp(recvBuff, "QUES|", 5) == 0) {
            // Tách chuỗi theo ký tự '|'
            // Format: QUES|Cau hoi|Op1|Op2|Op3|Op4
            char *token = strtok(recvBuff, "|"); // Lấy chữ QUES (bỏ qua)
            
            char *question = strtok(NULL, "|");
            char *op1 = strtok(NULL, "|");
            char *op2 = strtok(NULL, "|");
            char *op3 = strtok(NULL, "|");
            char *op4 = strtok(NULL, "\n"); // Cái cuối cùng có thể dính \n

            if (question && op1 && op2 && op3 && op4) {
                printf("\n[QUESTION]: %s\n", question);
                printf("1. %s\n", op1);
                printf("2. %s\n", op2);
                printf("3. %s\n", op3);
                printf("4. %s\n", op4);
                
                // 3. Nhập câu trả lời
                int ans;
                do {
                    printf("Enter your answer (1-4): ");
                    if (scanf("%d", &ans) != 1) {
                        while(getchar() != '\n'); // Xóa bộ đệm nếu nhập sai chữ
                        ans = 0;
                    }
                } while (ans < 1 || ans > 4);
                
                // 4. Gửi về server
                snprintf(sendBuff, sizeof(sendBuff), "ANSWER %d\n", ans);
                send(sockfd, sendBuff, strlen(sendBuff), 0);
                printf("Da gui dap an: %d. Dang cho ket qua...\n", ans);
            }
        } 
        else {
            // Có thể là thông báo kết quả hoặc loại khỏi cuộc chơi
            // Chúng ta sẽ xử lý ở Bước 4 (Xử lý kết quả)
            printf("[SERVER]: %s\n", recvBuff);
            // Nếu nhận được mã kết thúc game thì break;
        }
    }
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

    int logged_in = 0;

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
    if (n > 0) {
        recvBuff[n] = 0;
        explain_code(recvBuff);
    }

    int choice;

    while (1) {

        menu(logged_in);

        if (scanf("%d", &choice) != 1) break;
        while (getchar()!='\n'); // clear stdin

        //   MENU CHUA �ANG NHAP
        if (!logged_in) {

            if (choice == 1) { // REGISTER
                char username[50], password[50];
                printf("Username: "); scanf("%49s", username); while(getchar()!='\n');
                printf("Password: "); scanf("%49s", password); while(getchar()!='\n');

                snprintf(sendBuff,sizeof(sendBuff),"REGISTER %s %s", username, password);
                send(sockfd, sendBuff, strlen(sendBuff),0);

                n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
                recvBuff[n] = 0;
                explain_code(recvBuff);
            }

            else if (choice == 2) { // LOGIN
                char username[50], password[50];
                printf("Username: "); scanf("%49s", username); while(getchar()!='\n');
                printf("Password: "); scanf("%49s", password); while(getchar()!='\n');

                snprintf(sendBuff,sizeof(sendBuff),"LOGIN %s %s", username, password);
                send(sockfd, sendBuff, strlen(sendBuff),0);

                n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
                recvBuff[n] = 0;
                explain_code(recvBuff);

                if (strcmp(recvBuff,"110") == 0) {
                    logged_in = 1; // login succeed
                }
            }

            else if (choice == 3) {
                puts("Exit client.");
                break;
            }

            else puts("Invalid choice!");
        }

        // MENU �� �ANG NHAP
        else {
        	
            if (choice == 1) { // JOIN
                snprintf(sendBuff,sizeof(sendBuff),"JOIN");
                send(sockfd, sendBuff, strlen(sendBuff),0);

                // ==== LAN 1: nhan 200 hoac loi ====
                n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
                recvBuff[n] = 0;
                explain_code(recvBuff);
                
                if (strcmp(recvBuff,"210") == 0) {
                    puts("Dang bat dau tro choi...");
                    gamePlay(sockfd);
                    continue;
                }
                if (strcmp(recvBuff,"200") != 0) {
                    continue;
                }

                // recv() se block den khi server gui 202 hoac 210
                n = recv(sockfd, recvBuff, sizeof(recvBuff)-1, 0);
                recvBuff[n] = 0;
                explain_code(recvBuff);
                if (strcmp(recvBuff,"210") == 0) {
                    puts("Dang bat dau tro choi...");
                    gamePlay(sockfd);
                }
               // Neu l� 202, client tu in "Kh�ng du nguoi choi"
               // v� sau d� tu quay lai menu (nhu v�ng lap ch�nh)
            }
            
            else if (choice == 2) { // LOGOUT
                snprintf(sendBuff,sizeof(sendBuff),"LOGOUT");
                send(sockfd, sendBuff, strlen(sendBuff),0);

                n = recv(sockfd, recvBuff, sizeof(recvBuff)-1,0);
                recvBuff[n] = 0;
                explain_code(recvBuff);

                if (strcmp(recvBuff,"120") == 0)
                    logged_in = 0;
            }
            
            else if (choice == 3) {
                puts("Exit client.");
                break;
            }
            else puts("Invalid choice!");
        }
    }
    close(sockfd);
    return 0;
}


