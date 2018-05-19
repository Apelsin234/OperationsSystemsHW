all: clean comp 

clean: 
	rm -f client server echo*

run:run_server run_client

run_client:
	./client 127.0.0.1 23456

run_server:
	./server 127.0.0.1 23456

comp:comp_server comp_client

comp_server:
	gcc server.c -Wall -fexceptions -fsanitize=address -g -o server # -lsocket -lnsl

comp_client:
	gcc client.c -Wall -fexceptions -fsanitize=address -g -o client # -lsocket -lnsl

pack: clean
	zip homework5.zip *	