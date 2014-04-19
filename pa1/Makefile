CC=gcc
OUTFILE=program
CFLAGS=-Wall -std=c99 -pedantic

all: clean install

install: main.o pipe.o ipc.o
	$(CC) $(CFLAGS) -o $(OUTFILE) main.o pipe.o ipc.o

main.o: main.c common.h pa1.h ipc.h pipe.h
	$(CC) $(CFLAGS) -c main.c
	
pipe.o: pipe.c pipe.h
	$(CC) $(CFLAGS) -c pipe.c
	
ipc.o: ipc.c pipe.h ipc.h
	$(CC) $(CFLAGS) -c ipc.c
	
clean:
	rm -f main.o pipe.o ipc.o $(OUTFILE)