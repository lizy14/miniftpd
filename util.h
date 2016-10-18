/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */



#include "sentences.h"

#include <getopt.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <arpa/inet.h>


#define equal(x, y) (strcmp((x), (y))==0)

int str_replace(char* str, char find, char replace){
    int counter = 0;
    while(*str){
        if (*str == find){
            *str = replace;
            counter ++;
        }
        str++;
    }
    return counter;
}

int socket_connect(int* connfd, char* ip_and_port){

    //parse "(127,0,0,1,233,233)"
    char target_ip[20];
    int target_port = 0;

    char* comma;
    comma = strrchr(ip_and_port, ',');
    int ip_low = atoi(comma + 1);
    *comma = 0;
    comma = strrchr(ip_and_port, ',');
    int ip_high = atoi(comma + 1);
    *comma = 0;

    target_port = ip_high * 256 + ip_low;

    str_replace(ip_and_port, ',', '.');
    strcpy(target_ip, ip_and_port);

    printf("Connecting to %s:%d\n", target_ip, target_port);


    struct sockaddr_in server_addr;

    if ((*connfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }


    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(target_port);
    if (inet_pton(AF_INET, target_ip, &server_addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (connect(*connfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    return 0;

}



int socket_bind_listen(int* listenfd, int port){

    struct sockaddr_in addr;
    if ((*listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    int yes = 1;
    if(setsockopt(*listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
        printf("Error setsockopt(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(*listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (listen(*listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }
    return 0;
}

int rtrim(char* str){
    int i = strlen(str) - 1;
    while(i>=0){
        if(strchr(" \r\n\t", str[i]) != NULL){
            str[i] = '\0';
        }
        i--;
    }
    return 0;
}

int parse_sentence(char* sentence, char* verb, char* parameter){
    char* space = strchr(sentence, ' ');
    if(space != NULL)
        *space = '\0';
    strcpy(verb, sentence);
    if(space != NULL)
        strcpy(parameter, space+1);
    else
        *parameter = '\0';
    rtrim(verb);
    rtrim(parameter);
    return 0;
}

int startsWith(char* sentence, char* word){
    while(*word){
        if(*sentence == '\0')
            return 0;
        if(*word != *sentence)
            return 0;
        word++;
        sentence++;
    }
    return 1;
}

int send_string(int s, char *str) {
    int len = strlen(str);
    int total = 0; // how many bytes we've sent
    int charsleft = len; // how many we have left to send
    int n;
    while(total < len) {
        n = send(s, str+total, charsleft, 0);
        if (n == -1){
            break;
        }
        total += n;
        charsleft -= n;
    }
    if(n == -1){
        printf("Error send(): %s(%d)\n", strerror(errno), errno);
    }else{
        printf("sent %d bytes\n", total);
    }
    return n==-1 ? -1 : total; // return -1 on failure, 0 on success
}

// terminate on connection close or newline character
int recv_line(int s, char *buf, int len) {
    int received = 0;
    int available = len;
    int n;
    while(available > 0){
        puts("waiting for trunk");
        n = recv(s, buf+received, available, 0);
        if(n == 0 || n == -1){
            break;
        }
        printf("received trunk of %d bytes\n", n);
        received += n;
        available -= n;
        if(*(buf+received-1) == '\n') {
            // TODO: containing, rather than terminating with
            // TODO: remove terminating newline?
            received --;
            buf[received] = '\0';
            break;
        }
    }
    if(n == -1){
        printf("Error recv(): %s(%d)\n", strerror(errno), errno);
    }else{
        printf("received %d bytes\n", received);
    }
    return n==-1 ? -1 : received;
}

void send_file(int s, char* filename) {
    FILE* f = fopen(filename, "rb");
    if(!f) {
        printf("Error fopen(), filename %s\n", filename);
        return;
    }
    char buf[8192];
    int n;

    do {
        n = fread(buf, 1, 8190, f);
        send(s, buf, n, 0);
    } while (n > 0);

    fclose(f);
}

void recv_file(int s, char* filename) {
    FILE* f = fopen(filename, "wb");
    int n;
    char buf[8192];

    do {
        n = recv(s, buf, 8190, 0);
        fwrite(buf, 1, n, f);
    } while (n > 0);

    fclose(f);
}
