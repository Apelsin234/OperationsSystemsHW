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
#ifdef __APPLE__

#include <sys/event.h>
#endif


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

#ifdef __linux__
void func(int sockfd) {
	char buff[MAX] ;
	int slave_sockets[1024] ;
	bzero(slave_sockets, sizeof(slave_sockets)); 

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
#elif __APPLE__

#define CONST_SIZE 1000
static struct kevent kevent_struct, event_list[CONST_SIZE];
void func(int socket_listener_descriptor) {
	int client_socket;
    int kqueue_descriptor = kqueue();
    if (kqueue_descriptor < 0) {
        handle_error("kqueue");
    }

    EV_SET(&kevent_struct, socket_listener_descriptor, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    if (kevent(kqueue_descriptor, &kevent_struct, 1, NULL, 0, NULL) < 0) {
        handle_error("kevent(listener)");
    }

    int event_identifiers[CONST_SIZE];

    while (1) {
        int events_count = kevent(kqueue_descriptor, NULL, 0, event_list, CONST_SIZE, NULL);
        if (events_count < 0) {
            return -1;
        }
        for (size_t i = 0; i < events_count; ++i) {
            event_identifiers[i] = event_list[i].ident;
        }

        for (size_t i = 0; i < events_count; ++i) {
            if (event_identifiers[i] == socket_listener_descriptor) {
                client_socket = accept(socket_listener_descriptor, NULL, NULL);
                if (client_socket < 0) {
                    handle_error("accept");
                }
                EV_SET(&kevent_struct, client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
                if (kevent(kqueue_descriptor, &kevent_struct, 1, NULL, 0, NULL) < 0) {
                    handle_error("kevent");
                }
                printf("подключились к клиенту\n");
            } else {
                client_socket = event_identifiers[i];

                message_size = recv(client_socket, buffer, 256, 0);
                if (message_size <= 0) {
                    close(client_socket);
                    break;
                }
                if (strcmp(buffer, "close server") == 0) {
                    printf("Получено Сообщение: %s\n", buffer);
                    printf("Закрываем сервер\n");
                    close(kqueue_descriptor);
                    close(socket_listener_descriptor);
                    close(client_socket);
                    return 0;
                }
                if (strcmp(buffer, "exit") == 0) {
                    printf("Получено Сообщение: %s\n", buffer);
                    printf("Закрываем подключение к клиенту\n");
                    close(client_socket);
                    continue;
                }
                printf("Получено Сообщение: %s\n", buffer);
                printf("Отправляю принятое сообщение клиенту\n");
                send(client_socket, buffer, message_size, 0);

                if (message_size <= 0) {
                    printf("Закрываем подключение к клиенту\n");
                    close(client_socket);
                }
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
	struct sockaddr_in serv_addr;
	sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd == -1) {
		handle_error("socket\n");
	}
	printf("Socket successfully created..\n");

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr(ip);
	serv_addr.sin_port = htons(port);

	if((bind(sockfd, (SA*)&serv_addr, sizeof(serv_addr))) < 0) {
		handle_error("bind\n");
	}	
	printf("Socket successfully binded..\n");
	set_nonblock(sockfd);
	if((listen(sockfd, SOMAXCONN)) != 0) {	
		printf("listen");
	}

	printf("Server listening..\n");
	func(sockfd);
	close(sockfd);
}
