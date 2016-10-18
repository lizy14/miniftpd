/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */

#include "util.h"

int authenticate(char* username, char* password){
	if(startsWith(username, anonymous_username))
		return 1;
	else
		return 0;
}

int serve_client(int sockfd){

	send_string(sockfd, greeting);

	int flag_username_provided = 0;
	int flag_user_authenticated = 0;
	char username[8192];
	int flag_pasv_mode = 0;
	int flag_port_mode = 0;
	int pasv_mode_fd;
	char port_mode_parameter[8192];



	while(1){
		//main loop per client
		char sentence[8192];
		int p = recv_line(sockfd, sentence, 8191);
		if (p < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		if(p == 0){
			close(sockfd);
			return 0;
		}
		sentence[p] = 0;
		printf("heard %d bytes: \n", p);
		puts(sentence);
		puts("");
		char verb[8192];
		char parameter[8192];
		parse_sentence(sentence, verb, parameter);
		printf("verb '%s', parameter '%s'\n", verb, parameter);


		if(equal(verb, "USER")){
			strcpy(username, parameter);
			flag_username_provided = 1;
			if(startsWith(username, anonymous_username)){
				send_string(sockfd, anonymous_accepted);
			}else{
				send_string(sockfd, username_accepted);
			}

		}else if(equal(verb, "PASS")){
			if(flag_username_provided){
				if(authenticate(username, parameter)){
					flag_user_authenticated = 1;
					send_string(sockfd, login_successful);
				}else{
					send_string(sockfd, login_failed);
				}
			}else{
				send_string(sockfd, need_username_before_password);
			}

		}else if(equal(verb, "SYST")){
			send_string(sockfd, syst_hard_coded);

		}else if(equal(verb, "TYPE")){
			if(equal(parameter, "I") || 1 /*TODO*/){
				send_string(sockfd, type_hard_coded);
			}

		}else if(equal(verb, "QUIT")){
			send_string(sockfd, good_bye);
			return;

		}else if(equal(verb, "PASV")){
			flag_pasv_mode = 1;
			int port = 23333;
			socket_bind_listen(&pasv_mode_fd, port);
			char response[200];
			sprintf(
				response,
				"227 Entering Passive Mode(%d,%d,%d,%d,%d,%d) \n",
				127,
				0,
				0,
				1,
				port / 256,
				port % 256
			);
			send_string(sockfd, response);

		}else if(equal(verb, "PORT")){
			flag_port_mode = 1;

		}else if(equal(verb, "RETR")){
			if(flag_port_mode){
				//TODO

			}else if(flag_pasv_mode){
				send_string(sockfd, begin_transfer);
				int pasv_mode_transfer_fd = accept(pasv_mode_fd, NULL, NULL);
				send_file(pasv_mode_transfer_fd, parameter);
				close(pasv_mode_transfer_fd);
				close(pasv_mode_fd);

			}else{
				send_string(sockfd, need_transfer_connection);
			}

		}else if(equal(verb, "STOR")){
			if(flag_port_mode){

			}else if(flag_pasv_mode){
				send_string(sockfd, begin_transfer);
				int pasv_mode_transfer_fd = accept(pasv_mode_fd, NULL, NULL);
				recv_file(pasv_mode_transfer_fd, parameter);
				close(pasv_mode_transfer_fd);
				close(pasv_mode_fd);

			}else{
				send_string(sockfd, need_transfer_connection);
			}

		}else{
			send_string(sockfd, not_supported);
		}

	}

	return 0;
}

int main(int argc, char **argv) {

	//establish listening and distribute incomming connections

    int listenfd, connfd;

    int p;
    int len;

	if(socket_bind_listen(&listenfd, 21) != 0){
		return;
	}

    while (1) {
        if ((connfd = accept(listenfd, NULL, NULL)) == -1) {
            printf("Error accept(): %s(%d)\n", strerror(errno), errno);
            continue;
        }else{
			//connection accepted
			int pid = fork();
			if(pid < 0){
				printf("Error fork()");
			}else if(pid == 0){
				close(listenfd);
				serve_client(connfd);
				close(connfd);
				return 0;
			}
			close(connfd);
		}
        close(connfd);
    }
    close(listenfd);
	return 0;
}
