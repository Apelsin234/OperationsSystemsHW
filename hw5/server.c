#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <fcntl.h>
#define MAX 128
#define SA struct sockaddr 

#define handle_error(msg) \
		{perror(msg); exit(1);}

void func(int sockfd) {
	char buff[MAX];
	int n;
	while (1) {
		bzero(buff, MAX);
		int k = 0;
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
		printf("From client: %s\t To client : ", buff);
		bzero(buff, MAX);
		n = 0;
		while((buff[n++] = getchar()) != '\n');
		buff[n] = '\0';
		k = 0;
		while(k != n){
			int u = write(sockfd, buff + k, n - k);
			if (u == -1){
				handle_error("write");
			}
			k += u;
		}
		if(strncmp("exit", buff, 4) == 0) {
			printf("Server Exit...\n");
			break;
		}	
	}
}


int main(int argc, char * argv[]) {
	if(argc != 3) {
		printf("expected 2 args: ip, port\n");
		return 0;
	}
	const char * ip = argv[1];
	int port = atoi(argv[2]);
	if (port < 1025 || port > 99999){
		printf("expected port from 1025 to 99999\n");
		return 0;
	}

	int sockfd, connfd;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if (sockfd == -1) {
		handle_error("socket\n");
	}
	printf("Socket successfully created..\n");

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);

	if((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		handle_error("bind\n");
	}
	else	
		printf("Socket successfully binded..\n");
	if((listen(sockfd, 5)) != 0) {	
		printf("listen");
	}
	
	printf("Server listening..\n");
	connfd = accept(sockfd, 0, 0 );
	if(connfd < 0) {
		printf("accept\n");
	}
	
	printf("server acccept the client...\n");
	func(connfd);
	close(sockfd);
	close(connfd);
}
