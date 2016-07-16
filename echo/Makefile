CC = gcc

all: server client

server: server.o
	$(CC) -o $@ $^
	rm -f $^

client: client.o
	$(CC) -o $@ $^
	rm -f $^

server.o: server.c echo.h
client.o: client.c echo.h

.c.o:
	$(CC) -Wall -g -c ${word 1, $^}

clean:
	rm -f server client
