/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */

#include "util.h"
int serve_client(int sockfd){

	send_string(sockfd, greeting);

	int flag_logged_in = 0;
	int flag_pasv_mode = 0;
	int flag_port_mode = 0;
	int port_id = 0;

	while(1){
		//main loop per client
		char sentence[8192];
		int p = recv_line(sockfd, sentence, 8191);
		if (p < 0) {
			printf("Error read(): %s(%d)\n", strerror(errno), errno);
			close(sockfd);
			return 1;
		}
		sentence[p] = 0;
		printf("heard %d bytes: `%s`\n", p, sentence);

		char verb[8192];
		char parameter[8192];
		parse_sentence(sentence, verb, parameter);
		printf("verb `%s`, parameter `%s`", verb, parameter);

		char response[9000];
		sprintf(response, "verb `%s`, parameter `%s`", verb, parameter);

		int n = send_string(sockfd, response);
		if (n < 0) {
			printf("Error write(): %s(%d)\n", strerror(errno), errno);
			return 1;
		}
	}

	return 0;
}

int main(int argc, char **argv) {

	//establish listening and distribute incomming connections

    int listenfd, connfd;
    struct sockaddr_in addr;

    int p;
    int len;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

	int yes = 1;
	if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
		printf("Error setsockopt(): %s(%d)\n", strerror(errno), errno);
        return 1;
	}


    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 6789;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        printf("Error bind(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (listen(listenfd, 10) == -1) {
        printf("Error listen(): %s(%d)\n", strerror(errno), errno);
        return 1;
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
