#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <ctype.h>
#include "head.h"
#define MAX 1000


int socket_connect(int port, char *host) {
    int sockfd;
    struct sockaddr_in dest_addr;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket create error!!!");
        return -1;
    }

    memset(&dest_addr, 0, sizeof(sockaddr_in));

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(port);
    dest_addr.sin_addr.s_addr = inet_addr(host);

    if (connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
        perror("connect perror:");
        return -1;
    }
    return sockfd;
}



int socket_create(int port) {
	struct sockaddr_in sock_addr;
	int sockfd, yes = 1;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket() error:");
		return -1;
	}

	memset(&sock_addr, 0, sizeof(sockaddr));

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(port);
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		close(sockfd);
		perror("setsockopt error");
	}
	if (bind(sockfd, (struct sockaddr *)&sock_addr, sizeof(sock_addr))) {
		perror("bind() error\n");
		close(sockfd);
		return -1;
	}

	if (listen(sockfd, 20) < 0) {
		close(sockfd);
		perror("listen error:");
		return -1;
	}
    return sockfd;
}

int socket_accept(int sock_listen) {
	int sockfd;
	struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);
	//socklen_t server_len = sizeof(server_addr);
	sockfd = accept(sock_listen, (struct sockaddr*)&client_addr, &len);
	if (sockfd < 0) {
		perror("accept () error\n");
		return -1;
	}
	printf("链接对方IP ：%s 端口: %d\n", inet_ntoa(client_addr.sin_addr), \
			ntohs(client_addr.sin_port));
	return sockfd;
}

int short_link(const char *filepath, const char *filename, int lsockfd) {
    FILE *fp = fopen(filepath, "r");
    int lock = fileno(fp);
	char buf[100] = "404";
    if (fp == NULL) {
        send(lsockfd, buf, strlen(buf), 0);
        return 0;
    }

    //sprintf(buf, "%d", 200);
    //send(lsockfd, buf, strlen(buf), 0);

	int listen = socket_create(9504);
	int sockfd = socket_accept(listen);

    if (listen == -1 || sockfd == -1) {
        send(lsockfd, buf, strlen(buf), 0);
        printf("短链接建立失败\n");
        fflush(stdout);
        return 0;
    }

    sprintf(buf, "%d", 200);
    send(lsockfd, buf, strlen(buf), 0);

    printf("短链接成功建立.....\n");
    fflush(stdout);


	printf("客户端发送:%s \n", filename);
	sprintf(buf, "%s", filename);
	send(sockfd , buf, strlen(buf) + 1, 0);
	memset(buf, 0, sizeof(char) * 100);
    flock(lock, LOCK_EX);     //上锁
	while (fgets(buf, 100, fp)) {
		send(sockfd, buf, strlen(buf), 0);
	}
	printf("短链接退出\n");
    flock(lock, LOCK_UN);   //解锁
	close(sockfd);
    close(listen);
    fclose(fp);


    flock(fp->_fileno, LOCK_EX);
    fp = fopen(filepath, "w+");
    flock(fp->_fileno, LOCK_UN);

    if (fp) fclose(fp);
	return 0;
}

void long_link(char *buffer, int sockfd) {
	int signal = atoi(buffer);
	if (signal <= 0) {
		return;
	}

	char buf[100];
    printf("收到master请求: %d \n", signal);
    fflush(stdout);
	switch (signal) {
		case 100:
		    short_link("./cpu.log", "cpu.log", sockfd);
			break;
		case 300:
			short_link("./mem.log", "mem.log", sockfd);
			break;
		case 500:
		    short_link("./disk.log", "disk.log", sockfd);
			break;
        case 600:
            short_link("./user.log","user.log" ,sockfd);
            break;
        case 700:
            short_link("./sys.log", "sys.log", sockfd);

	}
}


void readLog(const char *bash, const char *filename) {
	int pid = fork();
    if (pid) return;
	char *buf = (char*)malloc(sizeof(char) * 11025);
	while (1) {
        FILE *f2 = fopen(filename, "a+");
        int lock = fileno(f2);
        flock(lock , LOCK_EX);    //上锁
	    int len = 0;
	    for (int i = 0; i < 12; i++) {
		    FILE *f1 = popen(bash, "r");
		    int temp = 0;
		    while (fgets(buf + len, 1024, f1)) {
			     len = strlen(buf);
		    }
        fclose(f1); 
        }
        printf("buf: %s\n", buf);
        fwrite(buf, sizeof(char), strlen(buf), f2);
        flock(lock, LOCK_UN);   //解锁 
        fclose(f2);
        sleep(20);
	}
	free(buf);
	exit(0);
}

int match_string(char *str,const char *target) {
    int len1 = strlen(str);
    int len2 = strlen(target);
    int *cnt = (int*)calloc(128, sizeof(int));
    for (int i = 0; i < 128; i++) {
        cnt[i] = len2 + 1;
    }

    for (int i = 0; target[i]; i++) {
        cnt[target[i]] = len2 - i;
    }

    int i = 0;
    while (i < len1) {
        int j = 0;
        for (; j < len2 && str[i + j] == target[j]; j++);
        if (j == len2) return 1;
        i += cnt[str[i + len2]];
    }
    return 0;
}

void ListenWarning(int port, char *host) {
    if (!fork())
    {
       while (1) {
           int sockfd = socket_connect(port, host);
           if (sockfd < 0) {
               sleep(5);
               continue;
           }
           FILE* fp = popen("bash sys.sh", "r");
           int lock = fileno(fp);
           char buf[1000];
           flock(lock, LOCK_EX);   //上锁
           fread(buf,sizeof(int),1000, fp);
           printf("%s \n", buf);
           fflush(stdout);
           if (match_string(buf, "warning")) {
               //printf("bei ya da ge \n");
               //fflush(stdout);
               send(sockfd, buf, strlen(buf), 0);
           }
           flock(lock, LOCK_UN);  //解锁
           if (fp) fclose(fp);
           close(sockfd);
           sleep(5);
       }
    }
}


int main() {
    char host_server[20] = "192.168.1.165";
    ListenWarning(60555, host_server);
	int listenfd = socket_create(5670);
    printf("listenfd = %d\n", listenfd);
	fflush(stdout);
	if (listenfd < 0) return 0;
	if (fork())
	{
	    while (1) {
	        int sockfd = socket_accept(listenfd);
	        if (sockfd < 0) {
		        printf("sockfd = %d", sockfd);
		        fflush(stdout);
	        }

	        char buffer[MAX];
	        while (recv(sockfd, buffer, MAX, 0) > 0) {
		        long_link(buffer, sockfd);
		        memset(buffer, 0, sizeof(char) * MAX);
	        }
	        printf("与master失去链接.....\n");
	    }
		exit(0);
	}

	readLog("bash cpu.sh", "./cpu.log");
    readLog("bash mem.sh", "./mem.log");
	readLog("bash disk.sh", "./disk.log");
    readLog("bash user.sh", "./user.log");
    readLog("bash sys.sh", "./sys.log");
	while(1) {
		int status;
		wait(&status);
	}
	return 0;
}
