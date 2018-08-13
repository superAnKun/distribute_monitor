#include <stdio.h>
#include <stdlib.h>
#include "head.h"
#define MAX 1000


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

	if (bind(sockfd, (struct sockaddr*)&sock_addr, sizeof(sock_addr))) {
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
	sockfd = accept(sock_listen, (struct sockaddr*)&client_addr, &len);
	if (sockfd < 0) {
		perror("accept () error\n");
		return -1;
	}
	return sockfd;
}


int socket_connect(int port, char *host) {
	int sockfd;
	struct sockaddr_in dest_addr;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket create error!!");
		return -1;
	}

	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = inet_addr(host);

	if (connect(sockfd, (struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0) {
		perror("connect perror:");
		return -1;
	}
	return sockfd;
}

void receive(int status, char *host) {
	char filepath[100] = {0};
	char buf[MAX];
	int ret = socket_connect(9504, host);
	if (ret < 0) return;

	if (recv(ret, buf, 50, 0) <= 0) return;
	sprintf(filepath, "./save/%s/%s", host, buf);
	printf("filepath : %s \n", filepath);
	fflush(stdout);

	FILE *fp = fopen(filepath, "a+");
	if (fp == NULL) {
		memset(filepath, 0, sizeof(filepath));
		sprintf(filepath, "%s/save/%s", getcwd(filepath, sizeof(filepath)), host);
		if (mkdir(filepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0) return;
	}
	memset(buf, 0, sizeof(char) * MAX);
	while (recv(ret, buf, MAX, 0) > 0) {
		printf("%s", buf);
		fwrite(buf, sizeof(char), strlen(buf), fp);
		memset(buf, 0, sizeof(char) * MAX);
	}

	close(ret);
	fclose(fp);
	printf("文件传输完毕 进程关闭\n");
	fflush(stdout);
}


void thread_child(int n, int clientid, char *host) {
	char buf[5][10] = {"100", "300", "500", "600", "700"};
	int i = 0;
	while(send(clientid, buf[i], strlen(buf[i]), 0) > 0 && i < 5){
		printf("发送client : %d  status: %s \n", n, buf[i]);
		fflush(stdout);
		char buffer[MAX];
	    if (recv(clientid, buffer, MAX, 0) <=0) {
		    printf("没有接受到  Client: %d 断开链接\n", n);
		    fflush(stdout);
		    break;
	    }
	    int status = atoi(buffer);
	    //printf("status: %d \n", status);
	    //fflush(stdout);
	    switch (status) {
		    case 404:
			    printf("client %d: %s file is not find !!\n", n, host);
			    fflush(stdout);
			    break;
		    case 200:
			    printf("client %d: %s file find success!!!!\n", n, host);
			    fflush(stdout);
			    receive(status, host);
			    break;
		    default:
			    printf("无效的请求\n");
			    fflush(stdout);
	    }
	    i++;
		printf("i = %d\n", i);
		fflush(stdout);
		//sleep(2);
	}

	printf("与Client : %d ip: %s失去链接... \n", n, host);
	fflush(stdout);
}

void listen_warning() {
	if (!fork()) {
	    int listen = socket_create(60555);
		while (1) {
	        int sockfd = socket_accept(listen);
			if (sockfd < 0) {
				printf("server_1 listen_warning!!!!\n");
				fflush(stdout);
				continue;
			}
			char buffer[1000];
			while (recv(sockfd, buffer, 1000, 0) > 0) {
				printf("%s \n", buffer);
				memset(buffer, 0, sizeof(char) * 1000);
			}
			close(sockfd);
		}
	    close(listen);
		exit(0);
	}
}


int main() {
	int n, ret;
	char host[100][20];
 //   listen_warning();
	while (1)
	{
		FILE *fp = fopen("./pilist", "r");
	    int i = 0;
	    while (fscanf(fp, "%s", host[i]) != EOF) {
			int pid = fork();
			if (!pid) {
				ret = socket_connect(5670, host[i]);
				thread_child(i, ret, host[i]);
				close(ret);
				exit(0);
			}
			i++;
	    }
		fclose(fp);
		sleep(30);
	}

	return 0;
}
