CC=gcc
OUTFILE=pa4
SOURCES=pipe.c main.c ipc.c pa2345.c
OBJ=pipe.o main.o ipc.o pa2345.o

CFLAGS=-Wall -pedantic -std=c99

LDPATH=-L.
LDLIBS=-lruntime

all: clean install

install: compile
	@echo "Installing pa2..."
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