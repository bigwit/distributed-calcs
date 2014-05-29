CC=gcc
OUTFILE=pa4
TMPDIR=pa4
OBJ=pipe.o main.o ipc.o pa2345.o
SOURCES=pipe.c main.c ipc.c pa2345.c
HEADERS=common.h ipc.h pa2345.h pipe.h

CFLAGS=-std=c99 -Wall -pedantic

TAR=$(OUTFILE).tar
TARGZ=$(TAR).gz

LDLIBS=-lruntime
LDPATH=-L.

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
	
clean: clean-obj
	@echo "Remove executable file..."
	rm -f $(OUTFILE)
	@echo ' '