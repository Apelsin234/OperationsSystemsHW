#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#include <unistd.h>

#define MAX 128
#define SA struct sockaddr
#define handle_error(msg) \
		{perror(msg); exit(1);}

void func(int sockfd) {
	char buff[MAX];
	int n;
	while (1) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while((buff[n++] = getchar()) != '\n');
		buff[n] = '\0';
		
		int k = 0;
		while(k != n){
			int u = write(sockfd, buff + k, n - k);
			if (u == -1){
				handle_error("write");
			}
			k += u;
		}
		bzero(buff, sizeof(buff));
		k = 0;
		while(1) {
			char bu_hp[MAX];
			int err = read(sockfd, bu_hp, sizeof(bu_hp));
			if (err == -1) {
				handle_error("read");
			}
			strncpy(buff + k, bu_hp, err);
			k += err;
			if(buff[k - 1] == '\n') {
				break;
			}
		}
		buff[k] = '\0';
		
		printf("From Server : %s", buff);
		if((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}

int main(int argc, char * argv[]) {
	if(argc != 3) {
		printf("expected 2 args: ip, port\n");
		return 1;
	}
	const char * ip = argv[1];
	int port = atoi(argv[2]);
	if (port < 1025 || port > 99999){
		printf("expected port from 1025 to 99999\n");
		return 0;
	}
	int sockfd;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET,SOCK_STREAM, 0);
	if (sockfd == -1) {
		handle_error("socket\n");
	}
	
	printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);
	if(connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0) {
		handle_error("connect\n");
		
	}
	
	printf("connected to the server..\n");
	func(sockfd);
	close(sockfd);
}