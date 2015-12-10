#  ______   _______ _____    ___  ____  ____  _____ ____  _ _ _
# | __ ) \ / /_   _| ____|  / _ \|  _ \|  _ \| ____|  _ \| | | |
# |  _ \\ V /  | | |  _|   | | | | |_) | | | |  _| | |_) | | | |
# | |_) || |   | | | |___  | |_| |  _ <| |_| | |___|  _ <|_|_|_|
# |____/ |_|   |_| |_____|  \___/|_| \_\____/|_____|_| \_(_|_|_)
#
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
	@echo "-- Linking and building $@..."
	@$(CC) $(CFLAGS) $(LIB) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "-- Compiling $@..."
	@$(CC) -c $(CLFAGS) $(INC) $(LIB) -o $@ $<

indent:
	indent -bad -bap -nbbb -sob -cdb -sc -br -ce -cdw -cli4 -nss -npcs -cs -nsaf -nsai -nsaw -npsl -brs -brf -i4 -ts4 $(SRCDIR)/*.c $(INCLUDEDIR)/*.h

clean:
	rm $(BUILDDIR)/*.o
	#rm $(TARGET)

force: clean $(TARGET)
