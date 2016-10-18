/*
   文件名:
   描　述:

   作　者: 李肇阳, 清华大学软件学院, lizy14@yeah.net
   创建于: 2016-10-18

   环　境: WSL, Windows 10.0.14393, gcc (Ubuntu 4.8.4-2ubuntu1~14.04.3) 4.8.4
 */

#include "util.h"

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in addr;
    char sentence[8192];
    int p;
    int len;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        printf("Error socket(): %s(%d)\n", strerror(errno), errno);
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
        }

		int p = recv_line(connfd, sentence, 8191);
        if (p < 0) {
            printf("Error read(): %s(%d)\n", strerror(errno), errno);
            close(connfd);
            continue;
        }
		sentence[p] = 0;
		len = strlen(sentence);
		printf("sentence %d", len);
		int i;
        for (i = 0; i < len; i++) {
            sentence[i] = toupper(sentence[i]);
        }

        int n = send_all(connfd, sentence, len);
        if (n < 0) {
            printf("Error write(): %s(%d)\n", strerror(errno), errno);
            return 1;
        }

        close(connfd);
    }

    close(listenfd);
}
