CC=g++

CFLAGS=-std=c++11 -c

LINK_THREAD=-lpthread

EFLAGS=-o


compile: server client

server: server.o
	$(CC) server.o $(EFLAGS) server $(LINK_THREAD)

server.o: server.cpp
	$(CC) $(CFLAGS) server.cpp

client: client.o
	$(CC) client.o $(EFLAGS) client 

client.o: client.cpp
	$(CC) $(CFLAGS) client.cpp



clean:
	rm -rf *.o server client 
