#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <fcntl.h>
#define MAX 128
#define SA struct sockaddr

#define handle_error(msg) \
		{perror(msg); exit(1);}

int set_nonblock(int fd) {
	int flags;
#if defined(O_NONBLOCK)
	if ((-1 == (flags = fcntl(fd, F_GETFL, 0)))) {
		flags = 0;
	}
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else 
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

void func(int sockfd, int slave_sockets[1024]) {
	char buff[MAX] ;

	bzero(buff, sizeof(buff));
	while (1) {
		fd_set sets;
		FD_ZERO(&sets);
		FD_SET(sockfd, &sets);
		for(int i = 0; i < 1024; i++) {
			if (slave_sockets[i] == 1)
				FD_SET(i, &sets);
		}
		int maxi = sockfd;
		for (int i = 0; i < 1024; i++) {
			if (slave_sockets[i] == 1 && i > maxi) {
				maxi = i;
			}
		}
		printf("%d\n", maxi );
		if (-1 == select(maxi + 1, &sets, NULL, NULL, NULL)) {
			handle_error("select");
		}
		for (int i = 0;i < 1024; i++) {
			if(slave_sockets[i] == 1 &&  FD_ISSET(i, &sets)) {
				int recvSize = recv(i, buff, sizeof(buff), 0);
				if (recvSize == 0 && errno != EAGAIN) {
					close(i);
					slave_sockets[i] = 0;
				} else if (recvSize != 0 ) {
					send(i, buff, recvSize, 0);
				}
			}
		}
		if (FD_ISSET(sockfd, &sets)) {
			int slavefd = accept(sockfd, 0, 0);
			if (slavefd == -1) {
				handle_error("accept");
			}
			set_nonblock(slavefd);
			slave_sockets[slavefd] = 1;
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
	int slave_sockets[1024] ;
	bzero(slave_sockets, sizeof(slave_sockets)); 

	int sockfd;
	struct sockaddr_in servaddr;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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
	printf("Socket successfully binded..\n");
	set_nonblock(sockfd);
	if((listen(sockfd, SOMAXCONN)) != 0) {	
		printf("listen");
	}
	
	printf("Server listening..\n");
	func(sockfd, slave_sockets);
	close(sockfd);
}
