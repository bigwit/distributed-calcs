install: main.o pipe.o ipc.o
	gcc -o program -std=c99 -Wall -pedantic main.o pipe.o ipc.o

main.o: main.c common.h pa1.h ipc.h pipe.h
	gcc -c -std=c99 -Wall -pedantic main.c
	
pipe.o: pipe.c pipe.h
	gcc -c -std=c99 -Wall -pedantic pipe.c
	
ipc.o: ipc.c pipe.h ipc.h
	gcc -c -std=c99 -Wall -pedantic ipc.c