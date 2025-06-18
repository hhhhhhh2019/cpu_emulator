OUT=build/main

DEBUG=1

LIBS=

CC?=gcc
LD=$(CC)

AS=rust_as

CC_FLAGS = -c -Wall -Werror
LD_FLAGS =

ifneq ($(LIBS),)
	CC_FLAGS += `pkg-config --cflags $(LIBS)`
	LD_FLAGS += `pkg-config --libs $(LIBS)`
endif

ifeq ($(DEBUG),1)
	CC_FLAGS += -g3 -DDEBUG
	LD_FLAGS += -g3
else
	CC_FLAGS += -DNDEBUG
endif

SOURCES = $(wildcard src/*.c)
OBJECTS = $(SOURCES:src/%.c=build/%.o)

BIOS_SOURCES = $(wildcard bios/*.S)

all: $(OBJECTS) bios
	$(LD) $(LD_FLAGS) $(OBJECTS) -o $(OUT)

build/%.o: src/%.c
	$(CC) $(CC_FLAGS) $^ -o $@

bios: $(SOURCES)
	$(AS) bios/bios.S bios.bin

clean:
	rm -f build/*
	rm -f $(OUT)
