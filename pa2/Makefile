CC=gcc
OUTFILE=pa2
CFLAGS=-std=c99 -Wall -pedantic
OBJ=pa23.o bank_robbery.o pipe.o ipc.o
SOURCES=pa23.c bank_robbery.c pipe.c ipc.c

LDLIBS=-lruntime
LDPATH=-L.

all: clean install

install: compile
	@echo "Installing pa2..."
	export LD_PRELOAD=/home/nikit/distributed-calcs/pa2/libruntime.so
	$(CC) $(CFLAGS) -o $(OUTFILE) $(OBJ) $(LDPATH) $(LDLIBS)
	@echo "=================================="
	@echo "BUILD SUCCESS"
	@echo "=================================="
	
compile: $(SOURCES)
	@echo "Compiling sources..."
	$(CC) $(CFLAGS) -c $(SOURCES)
	@echo ' '
	
clean-obj:
	@echo "Remove object files..."
	rm -f $(OBJ)
	@echo ' '
	
clean: clean-obj
	@echo "Remove executable file..."
	rm -f $(OUTFILE)
	@echo ' '