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

#ifdef __linux

#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif


#define MAX 1000
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

int send_safe(int socket, void *buffer, size_t length) {
    char *ptr = (char *) buffer;
    while (length > 0) {
        int i = send(socket, ptr, length, 0);
        if (i < 1) {
            return 1;
        }
        ptr += i;
        length -= i;
    }
    return 0;
}

int get_safe(int socket_fd, char *message) {
    char data[100];
    ssize_t data_read;

    while ((data_read = recv(socket_fd, data, 100, 0)) > 0) {
        for (int i = 0; i < data_read; i++) {
            *message++ = data[i];
        }
        if (data[data_read - 1] == '\n') {
            break;
        }
        bzero(data, 100);
    }

    if (data_read == -1 && errno != 0) {
        return 1;
    }

    *message = '\0';

    return 0;
}



#ifdef __linux__
void func(int sockfd) {
	int efd = epoll_create(100);
    struct epoll_event listenev;
    listenev.events = EPOLLIN | EPOLLPRI | EPOLLET;
    listenev.data.fd = sockfd;
    if (epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &listenev) < 0) {
        handle_error("epoll_ctl");
    }
    socklen_t client;
    struct epoll_event events[100];
    struct epoll_event connev;
    struct sockaddr_in cliaddr;

    while (1) {
        int nfds = epoll_wait(efd, events, 100, -1);

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == sockfd) {
                client = sizeof(cliaddr);
                int connfd = accept(sockfd, (struct sockaddr *) &cliaddr, &client);
                if (connfd < 0) {
                    perror("accept");
                    continue;
                }

                set_nonblock(connfd);
                connev.data.fd = connfd;
                connev.events = EPOLLIN | EPOLLOUT ;
                if ((!epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &connev)) < 0) {
                    perror("Epoll fd add");
                    close(connfd);
                    continue;
                }

            } else {
                int fd = events[n].data.fd;

                if (events[n].events & EPOLLIN) {

                    char reqline[MAX];

                    bzero(reqline, MAX);

                    if (get_safe(fd, reqline)) {
                        perror("Read");
                        break;
                    } else {
                        printf("Полученно сообщение: %s", reqline);
                    }

                    char response[MAX];
                    bzero(response, MAX);
                    sprintf(response, "Hello, %s", reqline);

                    if (send_safe(fd, response, strlen(response))) {
                        perror("Write");
                    } else {
                        printf("Отправлен ответ: %s", response);
                    }

                    epoll_ctl(efd, EPOLL_CTL_DEL, fd, &connev);
                    close(fd);
                }
            }
        }
}
}
#elif __APPLE__

#define CONST_SIZE 1000
static struct kevent kevent_struct, event_list[CONST_SIZE];
void func(int socket_listener_descriptor) {
	int client_socket, message_size;
    char buffer[256];
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
            return;
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

                message_size = get_safe(client_socket, buffer, 256);

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
                    return;
                }
                if (strcmp(buffer, "exit") == 0) {
                    printf("Получено Сообщение: %s\n", buffer);
                    printf("Закрываем подключение к клиенту\n");
                    close(client_socket);
                    continue;
                }
                printf("Получено Сообщение: %s\n", buffer);
                printf("Отправляю принятое сообщение клиенту\n");
				if(send_safe(client_socket, buffer, message_size) == -1){
					perror("send");
				}

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

	#ifdef __linux__
		int yeah = 1;
		if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yeah, sizeof(int))){
			handle_error("dasd");
		}
	#endif

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
