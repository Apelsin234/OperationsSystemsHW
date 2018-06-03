#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>  
#include <sys/uio.h>
#include <sys/wait.h>
#include <alloca.h>
#include <errno.h>


#define SIZE 2048
#define handle_error(msg) \
	{perror(msg); exit(10);}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "expected %s [serv_name]\n", argv[0]);
		exit(1);
	}
	int server_sockfd;
	int client_sockfd;
	struct sockaddr_un server_addr;
	struct sockaddr_un client_addr;

	char *server_name = argv[1];
	unlink(server_name); 
	if ((server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		handle_error("Socket failed in server");
	}

	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, server_name);

	if (bind(server_sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		handle_error("Bind failed in server");
	}

	

	listen(server_sockfd, 5);
	printf("Server waiting\n");
	while(1) {
		int client_len = sizeof(client_addr);
		client_sockfd = accept(server_sockfd, 
			(struct sockaddr *) &client_addr, &client_len);

		if (client_sockfd < 0) {
			handle_error("Accept failed in server");
		}

		char buffer[SIZE];
		
		if (read(client_sockfd, buffer, SIZE) < 0) {
			handle_error("Error getting pipe's name");
		}
		char pipe_name[100];
		strncpy(pipe_name, buffer, strlen(buffer));

		if (strncmp(pipe_name, "exit", sizeof(pipe_name)) == 0) {
			close(client_sockfd);
			break;
		}

		if (mkfifo(pipe_name, 0777) < 0) {
			handle_error("Failed makeFIFO");
		}

		printf("%s is created\n", pipe_name);

		int fd;
		if ((fd = open(pipe_name, O_RDWR)) <= 0) {

			remove(pipe_name);
			handle_error("Failed open FIFO");
		}
		printf("%s is opened\n", pipe_name);

		struct iovec vector;
		struct msghdr msg;
		struct cmsghdr *cmsg;

		vector.iov_base = pipe_name;
		vector.iov_len = strlen(pipe_name) + 1;

		msg.msg_name = NULL;
		msg.msg_namelen = 0;
		msg.msg_iov = &vector;
		msg.msg_iovlen = 1;

		cmsg = alloca(sizeof(struct cmsghdr) + sizeof(fd));
		cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
		cmsg->cmsg_level = SOL_SOCKET;
		cmsg->cmsg_type = SCM_RIGHTS;

		memcpy(CMSG_DATA(cmsg), &fd, sizeof(fd));

		msg.msg_control = cmsg;
		msg.msg_controllen = cmsg->cmsg_len;

		printf("%s\n", "Server send fd to client");
		if (sendmsg(client_sockfd, &msg, 0) != vector.iov_len) {

			remove(pipe_name);
			handle_error("Sendmsg failed");
		}

		close(client_sockfd);

		memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));


		char answer[SIZE];
		strcpy(answer, "Hello, bro :D");
		if (write(fd, answer, strlen(answer)) < 0) {
			handle_error("Error writing in server");
		}
		close(fd);
		
		
	}
	close(server_sockfd);
	return 0;
}