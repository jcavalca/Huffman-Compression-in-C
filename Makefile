CC = gcc

CFLAGS = -Wall -ansi -pedantic -g


all: hencode.c htable.h
	$(CC) $(CFLAGS) -o hencode hencode.c
hencode: hencode.c htable.h
	make all
run: hencode test
	./hencode test
clean: 
	rm hencode

