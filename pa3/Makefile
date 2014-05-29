CC=gcc
OUTFILE=pa3
TMPDIR=pa3
CFLAGS=-std=c99 -Wall -pedantic
OBJ=pa23.o bank_robbery.o pipe.o ipc.o lamport.o
SOURCES=pa23.c bank_robbery.c pipe.c ipc.c lamport.c
HEADERS=banking.h common.h ipc.h lamport.h pa2345.h pipe.h

TAR=$(OUTFILE).tar
TARGZ=$(TAR).gz

LDLIBS=-lruntime
LDPATH=-L.

all: clean install

install: compile
	@echo "Installing pa3..."
	export LD_PRELOAD=/home/nikit/distributed-calcs/pa2/libruntime.so
	$(CC) $(CFLAGS) -o $(OUTFILE) $(OBJ) $(LDPATH) $(LDLIBS)
	@echo "=================================="
	@echo "BUILD SUCCESS"
	@echo "=================================="
	
compile: $(SOURCES)
	@echo "Compiling sources..."
	$(CC) $(CFLAGS) -c $(SOURCES)
	@echo ' '
	
package: clean clean-package
	@echo "Move source files to temp dir..."
	mkdir $(TMPDIR)
	cp $(SOURCES) $(HEADERS) $(TMPDIR)
	@echo ' '
	@echo "Packaging on tar..."
	tar cvf $(TAR) $(TMPDIR)
	@echo ' '
	@echo "Compressing on gzip..."
	gzip -c $(TAR) > $(TARGZ)
	@echo ' '
	@echo "Remove temp files..."
	rm -rf $(TAR) $(TMPDIR)
	@echo ' '
	@echo "=================================="
	@echo "BUILD SUCCESS"
	@echo "=================================="
	
clean-obj:
	@echo "Remove object files..."
	rm -f $(OBJ)
	@echo ' '
	
clean-package:
	@echo "Remove packages..."
	rm -f $(TAR) $(TARGZ)
	@echo ' '
	
clean: clean-obj clean-package
	@echo "Remove executable file..."
	rm -f $(OUTFILE)
	@echo ' '
