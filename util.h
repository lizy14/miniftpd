/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */
#include "sentences.h"

#include <sys/socket.h>
#include <netinet/in.h>

#include <unistd.h>
#include <errno.h>

#include <string.h>
#include <memory.h>
#include <stdio.h>
#include <ctype.h>


#define equal(x, y) (strcmp((x), (y))==0)

int rtrim(char* str){
    int i = strlen(str) - 1;
    while(i>=0){
        if(strchr(" \r\n\t", str[i]) != NULL){
            str[i] = '\0';
        }
        i--;
    }
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
