CC=gcc
OUTFILE=pa2
CFLAGS=-std=c99 -Wall -pedantic -L. -lruntime
OBJ=pa23.o bank_robbery.o pipe.o ipc.o

install: pa23.o bank_robbery.o pipe.o ipc.o
	@echo "Installing pa2..."
	$(CC) $(CFLAGS) -o $(OUTFILE) $(OBJ)
	@echo "=================================="
	@echo "BUILD SUCCESS"
	@echo "=================================="

pa23.o: pa23.c banking.h ipc.h
	@echo "Compiling sources from pa23.c..."
	$(CC) $(CFLAGS) -c pa23.c
	
bank_robbery.o: bank_robbery.c banking.h
	@echo "Compiling sources from bank_robbery.c..."
	$(CC) $(CFLAGS) -c bank_robbery.c
	
pipe.o: pipe.c pipe.h
	@echo "Compiling sources from pipe.c..."
	$(CC) $(CFLAGS) -c pipe.c
	
ipc.o: ipc.c pipe.h ipc.h
	@echo "Compiling sources from ipc.c..."
	$(CC) $(CFLAGS) -c ipc.c
	
clean-obj:
	@echo "Remove object files..."
	rm -f $(OBJ)
	
clean: clean-obj
	@echo "Remove executable file..."
	rm -f $(OUTFILE)