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
char* connect_success = "Successfully logged in FTP server at %s as anonymous.\n";
char* connect_fail = "Failed to connect to %s\n";
char* user_command = "USER anonymous\n";
char* pass_command = "PASS zhaoyang_s_simple_ftp_client\n";

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
    printf("Passive mode, connecting to port %d\n", port);
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
        printf("Unexpected response: `%s`. Expecting `%s`. Aborting\n", rtrim(buffer), expectation);
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {

    int sockfd;
    char sentence[8192];

    char server[] = "127.0.0.1:21";
    if(socket_connect(&sockfd, server) != 0){
        printf(connect_fail, server);
        return 1;
    }


    if(send_checkresponse(sockfd, "", "220", sentence, 8191))
        return 1;
    if(send_checkresponse(sockfd, user_command, "331", sentence, 8191))
        return 1;
    if(send_checkresponse(sockfd, pass_command, "230", sentence, 8191))
        return 1;

    printf(connect_success, server);
    while(1) {
        input("command", sentence, 4096);

        char verb[8192];
        char parameter[8192];
        char ip_and_port[200];
        char command[9000];
        int transfd;

        parse_sentence(sentence, verb, parameter);
        printf_verbose("verb '%s', parameter '%s'\n", verb, parameter);

        if(equal(verb, "download")){
            if(send_checkresponse(sockfd, "PASV\n", "227", sentence, 8191))
                continue;
            int port = get_port_from_pasv_response(sentence);
            sprintf(ip_and_port, "127.0.0.1:%d", port);
            sprintf(command, "RETR %s\n", parameter);
            socket_connect(&transfd, ip_and_port);
            send_checkresponse(sockfd, command, "150", sentence, 8191);
            sleep(1);
            recv_file(transfd, parameter);
            close(transfd);
            send_checkresponse(sockfd, "", "226", sentence, 8191);
            continue;

        }else if(equal(verb, "upload")){
            if(send_checkresponse(sockfd, "PASV\n", "227", sentence, 8191))
                continue;
            int port = get_port_from_pasv_response(sentence);
            sprintf(ip_and_port, "127.0.0.1:%d", port);
            sprintf(command, "STOR %s\n", parameter);
            socket_connect(&transfd, ip_and_port);
            send_checkresponse(sockfd, command, "150", sentence, 8191);
            sleep(1);
            send_file(transfd, parameter);
            close(transfd);
            send_checkresponse(sockfd, "", "226", sentence, 8191);
            continue;

        }else if(equal(verb, "help")){
            printf("%s", usage);
            continue;
        }else if(equal(verb, "exit")){
            send_checkresponse(sockfd, "QUIT\n", "221", sentence, 8191);
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
