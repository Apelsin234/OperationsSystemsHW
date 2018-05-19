all: clean comp_serv comp_client

clean: 
	rm -f client server

comp_serv:
	cc -o server -Wall -fexceptions -fsanitize=address -g server.c

comp_client:
	cc -o client -Wall -fexceptions -fsanitize=address -g client.c

run:
	./server 127.0.0.1 23456 &
	./client 127.0.0.1 23456
pack: clean
	zip homework6.zip *