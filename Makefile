OUT=build/main

DEBUG=1

LIBS=

CC?=gcc
LD=gcc

CC_FLAGS = -c
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

build/%.o: src/%.c
	$(CC) $(CC_FLAGS) $^ -o $@

all: $(SOURCES)
	$(LD) $(LD_FLAGS) $^ -o $(OUT)

clean:
	rm -f build/*
	rm -f $(OUT)
