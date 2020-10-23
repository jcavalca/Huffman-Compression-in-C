CC = gcc

CFLAGS = -Wall -ansi -pedantic -g


all: hencode hdecode htable.h
hencode: hencode.c htable.h
	$(CC) $(CFLAGS) -o hencode hencode.c
hdecode: hdecode.c htable.h
	$(CC) $(CFLAGS) -o hdecode hdecode.c
run: hencode hdecode testfile
	./hencode testfile testfile.huff
	./hdecode testfile.huff testfile.out
clean: 
	rm hencode
	rm hdecode
	rm testfile.out
	rm testfile.huff

