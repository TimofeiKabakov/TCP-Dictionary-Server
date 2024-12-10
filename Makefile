.PHONY: clean

CC = gcc
CFLAGS = -g
CPPFLAGS = -std=gnu90 -Wall -Wextra -pedantic

all: client server

client: client.o
	$(CC) -o client $(CFLAGS) $(CPPFLAGS) client.o

client.o: client.c
	$(CC) -o client.o -c $(CFLAGS) $(CPPFLAGS) client.c

server: server.o
	$(CC) -o server $(CFLAGS) $(CPPFLAGS) server.o

server.o: server.c
	$(CC) -o server.o -c $(CFLAGS) $(CPPFLAGS) server.c

clean:
	rm *.o client server
