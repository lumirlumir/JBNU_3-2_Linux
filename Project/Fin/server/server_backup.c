#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 3240
#define IP "127.0.0.1"
#define BUFFSIZE 1024
#define TIME 1

/* Function */
void p(char* str) {
    sleep(TIME);

    printf("S : %s\n", str);
}

int main(void) {
/* Init */
    /* Init_Server */
    int s_sock;
    struct sockaddr_in s_addr = {AF_INET, htons(PORT), inet_addr(IP)};
    
    /* Init_Client */
    int c_sock;
    struct sockaddr_in c_addr;
    socklen_t c_addr_size = sizeof(struct sockaddr);

    /* Init_Buffer */
    char buf[BUFFSIZE] = {0};

    /* Init_File */
    FILE* fp = NULL;
    int fread_cnt = 0;

/* IPC_Socket_StartRoutine */
    /* 1_Socket */
    if((s_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error : 1_Socket ");
        exit(1);
    }
    int option = 1; setsockopt(s_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    /* 2_Bind */
    if(bind(s_sock, (struct sockaddr *)&s_addr, sizeof(struct sockaddr)) == -1) {
        perror("Error : 2_Bind ");
        exit(1);
    }

    /* 3_Listen */
    listen(s_sock, 1);

/* IPC_Socket_AcceptRoutine */
    for(int i = 0; ; i++) {
        /* 1_Accept */
        p("1_Accept Start(Wating for a client...)(Exit for Ctrl^C(SIGINT))");

        if((c_sock = accept(s_sock, (struct sockaddr *)&c_addr, &c_addr_size)) == -1) {
            perror("Error : 1_Accept ");
            exit(1);
        }

        printf("\n*********[Connecting Information]*********\n");
        printf("<SERVER>\n[INFO_SOCK]\ns_sock = %d\nc_sock = %d\n[INFO_CLIENT]\nClient Number = %d\nClient IP Address = %s\nPort = %d\n", 
            s_sock, c_sock, i, inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        printf("******************************************\n\n");

        /* 2_Send_IsConnected */
        p("2_Send_IsConnected Start");

        if(send(c_sock, "t", sizeof("t"), 0) == -1) {
            perror("Error : 2_Send_IsConnected ");
            exit(1);
        }

        p("2_Send_IsConnected End");

        /* 3_Recv_FileName */
        p("3_Recv_FileName Start");

        if(recv(c_sock, buf, BUFFSIZE, 0) == -1) {
            perror("Error : 3_Recv_FileName ");
            exit(1);
        }

        p("3_Recv_FileName End");
        printf("\n*** S : Client Says = %s ***\n\n", buf);

        /* 4_File_Open */
        if((fp = fopen(buf, "rb")) == NULL) { //����
            send(c_sock, "f", sizeof("f"), 0); //false(fail)
            perror("Error : 4_File_Open ");
            exit(1);
        }
        else { //����
            send(c_sock, "t", sizeof("t"), 0); //true(success)
        }

        /* 5_File_Send */
        while((fread_cnt = fread(buf, 1, 1, fp)) == 1)
            send(c_sock, buf, fread_cnt, 0);

        /* 6_Close */
        fclose(fp);
        close(c_sock);
    }
  
/* IPC_Socket_CloseRoutine */
    /* 1_Close */
    close(s_sock);

    /* 2_Return */
    return 0;
}

/*
1. 
send(c_sock, "1", sizeof("1"), 0) �� �ϰԵǸ�, server->client�� ���ۿ� '1'�� '\0'�̶�� 2���� ���ڰ� �Ѿ�� �ȴ�.
(���� '1'�̶�� char ���� �ϳ��� ������ �;��µ�, '1'�� '\0' 2���� ���ڰ� �Ѿ�� ��Ȳ.)
����, client�ʿ��� 1byte�� ���ڸ� receive�ϴ� ������� recv�Լ��� �����ϸ�,
'1'�� client���� ��������, '\0'�� server->client�� ���ۿ� ���� ������ �����.
*/