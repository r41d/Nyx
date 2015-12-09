CC := gcc
CFLAGS := -g -Wall -Wextra -std=c99

SRCDIR := src
BUILDDIR := build
INCLUDEDIR := include
EXEDIR := bin
TARGET := $(EXEDIR)/tcp_ip_stack

SOURCES := $(patsubst src/%,%,$(wildcard $(SRCDIR)/*.c))
OBJECTS := $(SOURCES:%.c=$(BUILDDIR)/%.o)

LIB := -lm
INC := -I $(INCLUDEDIR)

$(TARGET): $(OBJECTS)
	@echo " Linking and building $@..."
	$(CC) $(CFLAGS) $(LIB) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo " Compiling $@..."
	$(CC) -c $(CLFAGS) $(INC) $(LIB) -o $@ $<

indent:
	indent -bad -bap -nbbb -sob -cdb -sc -br -ce -cdw -cli4 -nss -npcs -cs -nsaf -nsai -nsaw -npsl -brs -brf -i4 -ts4 $(SRCDIR)/*.c $(INCLUDEDIR)/*.h

clean:
	rm $(BUILDDIR)/*.o
	rm $(EXEDIR)/*

print:
	echo $(SOURCES)
	echo $(OBJECTS)
