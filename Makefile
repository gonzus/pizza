.DEFAULT_GOAL := all

NAME = pizza

CC = cc
# CC = clang
LD = cc

AFLAGS += -std=c11
AFLAGS += -g
AFLAGS += -fsanitize=undefined
AFLAGS += -fsanitize=address   # cannot be used with thread
# AFLAGS += -fsanitize=thread  # cannot be used with address
# AFLAGS += -fsanitize=memory  # not supported on M1

CFLAGS += $(AFLAGS)
CFLAGS += -c
CFLAGS += -Wall -Wextra -Wshadow -Wpedantic
# CFLAGS += -D_GNU_SOURCE -D_XOPEN_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS += -I./inc/pizza -I/usr/local/include

# see more log messages
CFLAGS += -DLOG_LEVEL=1

# CFLAGS += -D_DEFAULT_SOURCE -D_GNU_SOURCE

CFLAGS += -g
# CFLAGS += -O3

LDFLAGS += $(AFLAGS)
LDFLAGS += -L. -L/usr/local/lib

LIBRARY = lib$(NAME).a

TEST_LIBS = -ltap -lz -lpthread

C_SRC_LIB = \
	stb.c \
	console.c \
	log.c \
	memory.c \
	slice.c \
	buffer.c \
	wedge.c \
	utf8.c \
	ymd.c \
	hms.c \
	timer.c \
	path.c \
	thrpool.c \
	mtwister.c \
	base64.c \
	uri.c \
	md5.c \
	blowfish.c \
	crypto.c \
	hash.c \
	deflator.c \
	regex.c \
	util.c \

C_OBJ_LIB = $(patsubst %.c, %.o, $(C_SRC_LIB))

.PHONY: first all tests test valgrind clean help

$(LIBRARY): $(C_OBJ_LIB)  ## (re)build library
	ar -crs $@ $^

C_SRC_TEST = $(wildcard t/*.c)
C_OBJ_TEST = $(patsubst %.c, %.o, $(C_SRC_TEST))
C_EXE_TEST = $(patsubst %.c, %, $(C_SRC_TEST))

%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

$(C_EXE_TEST): %: %.o $(LIBRARY)
	$(LD) $(LDFLAGS) -o $@ $^ $(TEST_LIBS)

tests: $(C_EXE_TEST)  ## (re)build all tests

test: tests ## run all tests
	@for t in $(C_EXE_TEST); do ./$$t; done

valgrind: tests  ## run all tests under valgrind
	@for t in $(C_EXE_TEST); do valgrind ./$$t 2>&1 | egrep -v '^==[0-9]+== (Memcheck,|Copyright |Using Valgrind|Command: |For lists of detected |[ \t]*($$|All heap blocks were freed|(HEAP|LEAK|ERROR) SUMMARY|total heap usage|in use at exit: 0 bytes|Process terminating with default action |(at|by) 0x))'; done

all: $(LIBRARY)  ## (re)build everything

clean:  ## clean everything
	rm -f *.o
	rm -f $(LIBRARY)
	rm -f $(C_OBJ_TEST) $(C_EXE_TEST)

help: ## display this help
	@grep -E '^[ a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort | awk 'BEGIN {FS = ":.*?# "}; {printf "\033[36;1m%-30s\033[0m %s\n", $$1, $$2}'
