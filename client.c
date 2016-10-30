/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */

//#define printf_verbose(...) //suppress trace output

#include "util.h"

char* usage = "Usage: \n    upload FILENAME\n    download FILENAME\n    exit\n    help\n";
char* what_did_you_say = "Unknown command. Try `help`.\n";
char* connect_success = "Successfully logged in FTP server at %s.\n";
char* connect_fail = "Failed to connect to %s\n";
char* user_command = "USER anonymousi\r\n";
char* pass_command = "PASS zhaoyang_s_simple_ftp_client\r\n";
char* passive_mode = "Passive mode, connecting to port %d\n";


typedef enum{
    PREFER_PASV,
    PREFER_PORT
} modePrefrence;

#include <sys/types.h>
#include <ifaddrs.h>


modePrefrence get_mode_prefrence(){
    struct ifaddrs* addrs;
    getifaddrs(&addrs);
    struct ifaddrs* tmp = addrs;
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET){
            struct sockaddr_in *pAddr = (struct sockaddr_in *)tmp->ifa_addr;
            if(startsWith(inet_ntoa(pAddr->sin_addr), "192.168.")){
                return PREFER_PASV;
            }
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(addrs);
    return PREFER_PORT;
}


int get_port_from_pasv_response(char* sentence){
    /*
        227 Entering Passive Mode(127,0,0,1,91,37)
     -> 23333
    */
    char* comma = strrchr(sentence, ',');
    if(comma == NULL)
        return -1;
    int low = atoi(comma + 1);
    *comma = '\0';
    char* last_comma = comma;
    comma = strrchr(sentence, ',');
    *last_comma = ',';
    if(comma == NULL)
        return -1;
    int high = atoi(comma + 1);
    int port = high * 256 + low;
    printf(passive_mode, port);
    return port;
}

int input(char* prompt, char* buffer, int limit){
    printf("\n%s>", prompt);
    fgets(buffer, limit, stdin);
    return 0;
}

int send_checkresponse(int sockfd, char* request, char* expectation, char* buffer, int buffer_size){
    if(strlen(request))
        send_string(sockfd, request);
    recv_line(sockfd, buffer, buffer_size);
    if(!startsWith(buffer, expectation)){
        printf_verbose("Unexpected response: `%s`.\nExpecting `%s`.\n", buffer, expectation);
        return 1;
    }
    return 0;
}

int pasv_file(int sockfd, char* filename, int flag_retr, char* sentence, int sentence_limit){
    char command[9000];
    int transfd;
    if(send_checkresponse(sockfd, "PASV\r\n", "227", sentence, sentence_limit))
        return 1;
    int port = get_port_from_pasv_response(sentence);
    char ip_and_port[200];
    sprintf(ip_and_port, "166.111.80.5:%d", port);
    sprintf(command, (flag_retr? "RETR %s\r\n": "STOR %s\r\n"), filename);
    socket_connect(&transfd, ip_and_port);
    send_checkresponse(sockfd, command, "150", sentence, sentence_limit);
    (flag_retr? recv_file: send_file)(transfd, filename);
    close(transfd);
    if(!send_checkresponse(sockfd, "", "226", sentence, sentence_limit)){
        printf("Successfully %sloaded `%s`.", (flag_retr? "down": "up"), filename);
        return 0;
    }
    return 1;
}


int port_file(int sockfd, char* filename, int flag_retr, char* sentence, int sentence_limit){
    char command[9000]; //buffer
    int transfd;

    int port = 23333 + rand() % 2333;
    sprintf(command, "PORT 101,5,216,201,%d,%d\r\n", port / 256, port % 256);
    if(send_checkresponse(sockfd, command, "227", sentence, sentence_limit))
        return 1;

    sprintf(command, (flag_retr? "RETR %s\r\n": "STOR %s\r\n"), filename);
    socket_bind_listen(&transfd, port);
    send_checkresponse(sockfd, command, "150", sentence, sentence_limit);
    (flag_retr? recv_file: send_file)(transfd, filename);
    close(transfd);
    if(!send_checkresponse(sockfd, "", "226", sentence, sentence_limit)){
        printf("Successfully %sloaded `%s`.", (flag_retr? "down": "up"), filename);
        return 0;
    }
    return 1;
}


int main(int argc, char **argv) {

    int sockfd;
    char sentence[8192];

    modePrefrence prefered_mode = get_mode_prefrence();


    //get server
    char server[233];
    input("Server IP and Port, defult: 127.0.0.1:21", server, 233);
    //printf("%d\n", (int)strlen(server));
    if(strlen(server) <= 1){
        strcpy(server, "127.0.0.1:21");
    }


    //build TCP connection
    if(socket_connect(&sockfd, server) != 0){
        printf(connect_fail, server);
        return 1;
    }
    //are you serving FTP?
    if(send_checkresponse(sockfd, "", "220", sentence, 8191))
        return 1;


    //credentials
    input("Login Username, defult: anonymous", sentence, 9191);

    if(strlen(sentence) <= 1 || startsWith(sentence, "anonymous")){
        //anonymous user
        if(send_checkresponse(sockfd, user_command, "331", sentence, 8191))
            return 1;
        if(send_checkresponse(sockfd, pass_command, "230", sentence, 8191))
            return 1;
    }else{
        char command[9000];
        sprintf(command, "USER %s\r\n", sentence);
        if(send_checkresponse(sockfd, command, "331", sentence, 8191))
            return 1;
        char* password = getpass("Login Password >");
        char* password_command = (char*)malloc((strlen(password)+233)*sizeof(char));
        sprintf(password_command, "PASS %s\r\n", password);
        if(send_checkresponse(sockfd, password_command, "230", sentence, 8191))
            return 1;
    }





    printf(connect_success, server);
    while(1) {
        input("command", sentence, 4096);

        char verb[8192];
        char parameter[8192];


        parse_sentence(sentence, verb, parameter);
        printf_verbose("verb '%s', parameter '%s'\n", verb, parameter);

        if(equal(verb, "download")){
            if(
                (prefered_mode==PREFER_PASV? pasv_file: port_file)
                    (sockfd, parameter, 1, sentence, 8190)
            ){
                (prefered_mode!=PREFER_PASV? pasv_file: port_file)
                    (sockfd, parameter, 1, sentence, 8190);
            }
            continue;

        }else if(equal(verb, "upload")){
            if(
                (prefered_mode==PREFER_PASV? pasv_file: port_file)
                    (sockfd, parameter, 0, sentence, 8190)
            ){
                (prefered_mode!=PREFER_PASV? pasv_file: port_file)
                    (sockfd, parameter, 0, sentence, 8190);
            }
            continue;

        }else if(equal(verb, "help")){
            printf("%s", usage);
            continue;
        }else if(equal(verb, "exit")){
            send_checkresponse(sockfd, "QUIT\r\n", "221", sentence, 8191);
            break;
        }else{
            printf("%s", what_did_you_say);
            continue;
        }
    }
    close(sockfd);
    printf("Bye!\n");

    return 0;
}
