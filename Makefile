NAME = pizza

# CFLAGS += -std=c89 -Wno-gcc-compat -Wno-comment
# CFLAGS += -std=c99
CFLAGS += -std=c11

# see more log messages
CFLAGS += -DLOG_LEVEL=1

# uncomment to make warnings into errors
# CFLAGS += -Werror

# uncomment to stop complaints about unused functions
# CFLAGS += -Wno-unused-function

CFLAGS += -I.
CFLAGS += -g
CFLAGS += -Wall -Wextra -Wshadow
CFLAGS += -Wpedantic
CFLAGS += -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE -D_GNU_SOURCE

LIBRARY = lib$(NAME).a

all: $(LIBRARY)

C_SRC_LIB = \
	log.c \
	memory.c \
	slice.c \
	buffer.c \
	wedge.c \
	utf8.c \
	ymd.c \
	hms.c \
	timer.c \
	thrpool.c \
	mtwister.c \
	hash.c \
	util.c \

C_OBJ_LIB = $(C_SRC_LIB:.c=.o)

$(LIBRARY): $(C_OBJ_LIB)
	ar -crs $@ $^

C_SRC_TEST = $(wildcard t/*.c)
C_OBJ_TEST = $(patsubst %.c, %.o, $(C_SRC_TEST))
C_EXE_TEST = $(patsubst %.c, %, $(C_SRC_TEST))

%.o: %.c
	cc $(CFLAGS) -c -o $@ $^

$(C_EXE_TEST): %: %.o $(LIBRARY)
	cc $(CFLAGS) $(LDFLAGS) -o $@ $^ -ltap -lpthread

tests: $(C_EXE_TEST)

test: tests
	@for t in $(C_EXE_TEST); do ./$$t; done

valgrind: tests
	@for t in $(C_EXE_TEST); do valgrind ./$$t 2>&1 | egrep -v '^==[0-9]+== (Memcheck,|Copyright |Using Valgrind|Command: |For lists of detected |[ \t]*($$|All heap blocks were freed|(HEAP|LEAK|ERROR) SUMMARY|total heap usage|in use at exit: 0 bytes|Process terminating with default action |(at|by) 0x))'; done

clean:
	rm -f *.o
	rm -f $(LIBRARY)
	rm -f $(C_OBJ_TEST) $(C_EXE_TEST)
