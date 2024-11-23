CC=gcc
C_INCLUDES:=$(shell pkg-config --cflags libbrotlicommon libbrotlidec libbrotlienc)
L_INCLUDES:=$(shell pkg-config --libs libbrotlicommon libbrotlidec libbrotlienc)
CFLAGS=-lz ${C_INCLUDES} ${L_INCLUDES}
LIBSOURCES = $(wildcard lib/*.c) 
SOURCES = $(LIBSOURCES) tools/filesize.c
OBJDIR = $(BINDIR)/obj
LIBOBJECTS = $(addprefix $(OBJDIR)/, $(LIBSOURCES:.c=.o))
OBJECTS = $(addprefix $(OBJDIR)/, $(SOURCES:.c=.o))
BINDIR = bin
EXECUTABLE = filesize
DIRS = $(OBJDIR)/lib $(OBJDIR)/tools

all: test
	@:

.PHONY: all clean test

$(DIRS):
	mkdir -p $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -lm -o $(BINDIR)/$(EXECUTABLE) $(CFLAGS)

lib: $(LIBOBJECTS)
	ar -crs $(LIBOBJECTS)

test: $(EXECUTABLE)
	$(BINDIR)/$(EXECUTABLE) Makefile Makefile

clean:
	rm -rf $(BINDIR)

.SECONDEXPANSION:
$(OBJECTS): $$(patsubst %.o,%.c,$$(patsubst $$(OBJDIR)/%,%,$$@)) | $(DIRS)
	$(CC) $(CFLAGS) $(CPPFLAGS) -Ilib \
        -c $(patsubst %.o,%.c,$(patsubst $(OBJDIR)/%,%,$@)) -o $@

.PHONY:
fmt:
	clang-format -i tools/filesize.c