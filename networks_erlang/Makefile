CC = gcc
CFLAGS = -g -Wall -I./

all: main findusers

main: erlang.c 
	$(CC) $(CFLAGS) $(LIBS) erlang.c -o $@ -lpthread


findusers: findusers.c
	$(CC) $(CFLAGS) $(LIBS) findusers.c -o $@ -lpthread
	
clean:
	rm erlang
