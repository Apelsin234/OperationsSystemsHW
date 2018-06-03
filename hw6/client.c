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
#ifdef __linux

#include <sys/epoll.h>
#elif __APPLE__
#include <sys/event.h>
#endif
#define MAX 1000
#define SA struct sockaddr
#define handle_error(msg) \
		{perror(msg); exit(1);}
#define handle_close(msg) \
		{close(epollfd); close(sockfd); handle_error(msg)}

void send_safe(int sock, char* buff, int len) {
	int nsend = 0;
	while (len > 0) {
        nsend = write(sock, buff + nsend, len);
        if (nsend < 0 && errno != EAGAIN) {
            close(sock);
            exit(0);
        }
        len -= nsend;
	}
}

void get_safe(int sock, char* buff) {
	bzero(buff, MAX);

	int n = 0;
	int nrecv = 0;

    while (1) {
    	nrecv = read(sock, buff + n, MAX - 1);
        if (nrecv == -1 && errno != EAGAIN) {
        	handle_error("read error!");

        }
		if ((nrecv == -1 && errno == EAGAIN) || nrecv == 0) {
        	return;
        }
        n += nrecv;
	}
}


#ifdef __linux
void func(int sockfd, int ok) {
	char buff[MAX];
	struct epoll_event event;
	int epollfd = epoll_create(10);
	event.events = EPOLLIN;
	if(!ok) {
		event.events |= EPOLLOUT;
	}
	event.data.fd = sockfd;

	int r = epoll_ctl(epollfd, EPOLL_CTL_ADD,sockfd,&event);
	if (r == -1) {
		handle_close("epoll_ctl");
	}

	
	int loop, epollout , epollin ;
	loop = epollout = epollin = 0;
	while(1) {
		struct epoll_event events[10];

		int n;
		if((n = epoll_wait(epollfd, events, 10, -1))== -1) {
			handle_close("epoll_wait");

		}
		if (loop == 1) {
			break;
		}
		for(int i = 0; i < n; i++) {
			if( (events[i].data.fd == sockfd ) && (events[i].events & EPOLLOUT) && (!ok) ) {
				
				int err = 0;
				socklen_t len = sizeof(int);
				int gi = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, &len);
				if (gi != -1) {
					if(err == 0) {
						struct epoll_event ev;
						ev.events = EPOLLOUT | EPOLLIN;
						ev.data.fd = sockfd;
						if(epoll_ctl(epollfd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
							handle_close("epoll_ctl 2");
						}
					} else {
						struct epoll_event ev;
						ev.events = 0;
						ev.data.fd = sockfd;
						epoll_ctl(epollfd, EPOLL_CTL_DEL, sockfd, &ev);
						close(sockfd);
					}
				}
			}
			if ((events[i].events & EPOLLOUT) && epollout == 0) {
                int n = 0;
                while((buff[n++] = getchar()) != '\n');
                buff[n] = '\0';
                
                send_safe(events[i].data.fd, buff, n);
                printf("Сообщение отправленно: %s", buff);
                epollout = 1;
            }

            if ((events[i].events & EPOLLIN) && epollin == 0) {
                
                get_safe(events[i].data.fd, buff);
                loop = 1;
                epollin = 1;
                printf("Ответ: %s\n", buff);
            }

		}
	}
	close(epollfd);

	
}
#elif __APPLE__

#define CONST_SIZE 1000
static struct kevent kevent_struct, event_list[CONST_SIZE];
void func(int socket_descriptor, int ok) {
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
	sockfd = socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (sockfd == -1) {
		handle_error("socket\n");
	}
	
	printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(ip);
	servaddr.sin_port = htons(port);
	int ok = 1;
	if((connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) == -1) && (errno == EINPROGRESS) ) {
		ok = 0;
		
	}
	
	printf("connected to the server..\n");
	func(sockfd, ok);
	close(sockfd);
}
