CC=gcc
OUTFILE=program
CFLAGS=-Wall -std=c99 -pedantic

all: install clean-obj

install: main.o pipe.o ipc.o
	$(CC) $(CFLAGS) -o $(OUTFILE) main.o pipe.o ipc.o
	
clean: clean-obj
	rm -f $(OUTFILE)
	
clean-obj:
	rm -f main.o pipe.o ipc.o
	
main.o: main.c common.h pa1.h ipc.h pipe.h
	$(CC) $(CFLAGS) -c main.c
	
pipe.o: pipe.c pipe.h
	$(CC) $(CFLAGS) -c pipe.c
	
ipc.o: ipc.c pipe.h ipc.h
	$(CC) $(CFLAGS) -c ipc.c