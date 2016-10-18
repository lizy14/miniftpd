/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */

#include "util.h"


int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in addr;
    char sentence[8192];
    int len;
    int p;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 6789;
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        printf("Error inet_pton(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        printf("Error connect(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    fgets(sentence, 4096, stdin);
    len = strlen(sentence);
    sentence[len] = '\n';
    sentence[len + 1] = '\0';

    int n = send_string(sockfd, sentence);
    if (n < 0) {
        printf("Error write(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    p = recv_line(sockfd, sentence, 8191);
    if (p < 0) {
        printf("Error read(): %s(%d)\n", strerror(errno), errno);
        return 1;
    }

	sentence[p - 1] = '\0';

	printf("FROM SERVER: `%s`\n", sentence);

	close(sockfd);

	return 0;
}
