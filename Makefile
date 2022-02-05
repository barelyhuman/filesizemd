CC=gcc
CCFLAGS=-lz -Ilib 
LIBSOURCES = $(wildcard lib/*.c) 
SOURCES = $(LIBSOURCES) tools/filesize.c
BINDIR = bin
EXECUTABLE = filesize

all: clean bin

bin:
	mkdir -p $(BINDIR)
	$(CC) $(CCFLAGS) $(SOURCES)  -lm -o $(BINDIR)/$(EXECUTABLE)

clean:
	rm -rf $(BINDIR)


