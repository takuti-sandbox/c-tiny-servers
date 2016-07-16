CC = gcc

all: server client

server: server.o
	$(CC) -o $@ $^
	rm -f $^

client: client.o
	$(CC) -o $@ $^
	rm -f $^

server.o: server.c
	$(CC) -Wall -g -c $^

client.o: client.c
	$(CC) -Wall -g -c $^ 

clean:
	rm -f server client
