#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include <unistd.h>
#ifdef __APPLE__
#include <sys/event.h>
#endif
#define MAX 128
#define SA struct sockaddr
#define handle_error(msg) \
		{perror(msg); exit(1);}

		int send_safe(int sock, char* buff, int len) {
	while(len > 0){
		int i = send(sock, buff, len ,0);
		if(i < 1) {
			return -1;
		}
		buff += i;
		len -= i;
	}
	return 0;
}

int get_safe(int sock, char* buff) {
	char data[100];
	int d_len;
	int k = 0;
	while((d_len = recv(sock, data, 100, 0)) > 0 ) {
		
		for(int i = 0; i < d_len; i++) {
			*buff++ = data[i];
		}
		k += d_len;
		if(data[d_len - 1] == '\n') {
			break;
		}

		bzero(data, 100);
	}

	if(k == 0 || (d_len == -1 && errno != 0)) {
		return -1; 
	}

	*buff ='\0';
	return d_len;
}


#ifdef __linux
void func(int sockfd) {
	char buff[MAX];
	int n;
	while (1) {
		bzero(buff, sizeof(buff));
		printf("Enter the string : ");
		n = 0;
		while((buff[n++] = getchar()) != '\n');
		buff[n] = '\0';
		
		if(send_safe(sockfd, buff, n) == -1){
			perror("send");
		}
			
		bzero(buff, sizeof(buff));
		if(get_safe(sockfd, buff) == -1){
			perror("recv");
			exit(1);
		}
		
		printf("From Server : %s", buff);
		if((strncmp(buff, "exit", 4)) == 0) {
			printf("Client Exit...\n");
			break;
		}
	}
}
#elif __APPLE__

#define CONST_SIZE 1000
static struct kevent kevent_struct, event_list[CONST_SIZE];
void func(int socket_descriptor) {
    char message[256];
    char buf[sizeof(message)];

	int kqueue_descriptor = kqueue();
    if (kqueue_descriptor < 0) {
        handle_error("kqueue");
    }
    EV_SET(&kevent_struct, STDIN_FILENO, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    // регистриция события
    if (kevent(kqueue_descriptor, &kevent_struct, 1, NULL, 0, NULL) < 0) {
        handle_error("STDIN_FILENO");
    }

    // то же самое, но уже про сокет
    EV_SET(&kevent_struct, socket_descriptor, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueue_descriptor, &kevent_struct, 1, NULL, 0, NULL) < 0) {
        handle_error("socket_descriptor");
    }

    int event_identifiers[CONST_SIZE];
    printf("Введите сообщение серверу (Для выхода - \"exit\"): ");
    while (1) {
        int events_count = kevent(kqueue_descriptor, NULL, 0, event_list, CONST_SIZE, NULL);
        if (events_count < 0) {
            return;
        }
        for (size_t i = 0; i < events_count; ++i) {
            event_identifiers[i] = event_list[i].ident;
        }

        for (size_t i = 0; i < events_count; ++i) {
            if (event_identifiers[i] == STDIN_FILENO) {
                int n = 0;
                while((message[n++] = getchar()) != '\n');
                message[n] = '\0';
                if (!strcmp(message, "exit")) {
                    printf("отправка сообщения о выходе на сервер...\n"); // нужно ли мне вообще это?
                    if(send_safe(socket_descriptor, message, sizeof(message))) {
                    	perror("send1");
                    }
                    close(socket_descriptor);
                    close(kqueue_descriptor);
                    return;
                }
                printf("отправка сообщения на сервер...\n");
                if(-1 == send_safe(socket_descriptor, message, sizeof(message))){
                	perror("send2");
                }
                printf("Ожидание сообщения\n");
            } else if (event_identifiers[i] == socket_descriptor) {
                if (get_safe(socket_descriptor, buf) <= 0) {
                    printf("Сервер был закрыт\n");
                    close(socket_descriptor);
                    close(kqueue_descriptor);
                    return;
                }
                printf("Получено сообщение: %s\n", buf);
            } else {
                handle_error("unexpected event identifier");
            }
        }
    }
}
#else
#error Incorrect sys
#endif

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
