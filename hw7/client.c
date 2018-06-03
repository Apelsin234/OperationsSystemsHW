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


#define SIZE 2048
#define handle_error(msg) \
	{perror(msg); exit(10);}

int main(int argc, char **argv) {
	if (argc != 3) {
		fprintf(stderr, "expected %s [serv_name][file_name]\n", argv[0]);
		exit(1);
	}
	int sockfd;
	struct sockaddr_un addr;
	char *name = argv[1];
	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		handle_error("Socket failed in client");
	}
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, name);

	if (connect(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		handle_error("Connecting failed in client");
	}
	char buffer[SIZE];

	char sendMessage[SIZE];
	char *pipe_name = argv[2];
	strcpy(sendMessage, pipe_name);

	if (write(sockfd, sendMessage, strlen(sendMessage)) < 0) {
		handle_error("Error sending pipe's name");
	}

	struct iovec vector;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	int fd;
	
	vector.iov_base = buffer;
	vector.iov_len = SIZE;

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vector;
	msg.msg_iovlen = 1;

	cmsg = alloca(sizeof(struct cmsghdr) + sizeof(fd));
	cmsg->cmsg_len = sizeof(struct cmsghdr) + sizeof(fd);
	msg.msg_control = cmsg;
	msg.msg_controllen = cmsg->cmsg_len;

		
	if (recvmsg(sockfd, &msg, 0) < 0) {
		handle_error("Recvmsg failed");
	} 
	memcpy(&fd, CMSG_DATA(cmsg), sizeof(fd));
	printf("%s\n", "Client got fd");

	if (read(fd, buffer, SIZE) < 0) {

    	remove(pipe_name);
    	handle_error("Read from file failed");
    }
	
	printf("%s\n", "Message from server:");
	
	
	printf("%s\n", buffer);
	
	close(sockfd);
	remove(pipe_name);
	return 0;
}
