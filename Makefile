# No. 1 mistake:
#  ______   _______ _____    ___  ____  ____  _____ ____  _ _ _
# | __ ) \ / /_   _| ____|  / _ \|  _ \|  _ \| ____|  _ \| | | |
# |  _ \\ V /  | | |  _|   | | | | |_) | | | |  _| | |_) | | | |
# | |_) || |   | | | |___  | |_| |  _ <| |_| | |___|  _ <|_|_|_|
# |____/ |_|   |_| |_____|  \___/|_| \_\____/|_____|_| \_(_|_|_)
#
CC := gcc
CFLAGS := -g -std=c99 -Wall # -Wextra

SRCDIR := src
PROGRAMDIR := program
BUILDDIR := build
INCLUDEDIR := include
EXEDIR := bin

SOURCES := $(patsubst $(SRCDIR)/%,%,$(wildcard $(SRCDIR)/*.c))
PROGRAMS := testing testserver testclient
PROGRAMSTARGET := $(patsubst %,$(EXEDIR)/%,$(PROGRAMS))
OBJECTS := $(filter-out $(patsubst %,$(BUILDDIR)/%.o,$(PROGRAMS)),$(SOURCES:%.c=$(BUILDDIR)/%.o))

.SECONDARY: $(OBJECTS)
.SECONDARY: $(patsubst %,$(BUILDDIR)/%.o,$(PROGRAMS))

LIB := -lm
INC := -I $(INCLUDEDIR)

nyx: $(PROGRAMSTARGET)

$(EXEDIR)/%: $(OBJECTS) $(BUILDDIR)/%.o
	@echo "-- Linking and building $@..."
	@$(CC) $(CFLAGS) $(LIB) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@echo "-- Compiling $@..."
	@$(CC) -c $(CFLAGS) $(INC) $(LIB) -o $@ $<

indent:
	indent -bad -bap -nbbb -sob -cdb -sc -br -ce -cdw -cli4 -nss -npcs -cs -nsaf -nsai -nsaw -npsl -brs -brf -i4 -ts4 $(PROGRAMDIR)/*.c $(SRCDIR)/*.c $(INCLUDEDIR)/*.h

rights:
	sudo setcap cap_net_raw+ep bin/testing
	sudo setcap cap_net_raw+ep bin/testserver
	sudo setcap cap_net_raw+ep bin/testclient

clean:
	-rm $(BUILDDIR)/*.o
	-rm $(PROGRAMSTARGET)

force: clean nyx rights

