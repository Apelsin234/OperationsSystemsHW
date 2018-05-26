all: clean comp_serv comp_client

clean: 
	rm -f client server homework6.zip

comp_serv:
	cc -o server -Wall -fexceptions -fsanitize=address -g server.c

comp_client:
	cc -o client -Wall -fexceptions -fsanitize=address -g client.c

run_server:
	./server 127.0.0.1 23456
run_client:
	./client 127.0.0.1 23456
pack: clean
	zip homework6.zip *