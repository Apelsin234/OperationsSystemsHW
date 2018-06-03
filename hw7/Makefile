all: createServer createClient

createServer: server.c
	gcc -o server server.c

createClient: client.c
	gcc -o client client.c
clean:
	rm -f server client

