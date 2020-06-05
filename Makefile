NAME = pizza

# see more log messages
CFLAGS += -DLOG_LEVEL=1

# uncomment to make warnings into errors
# CFLAGS += -Werror

# uncomment to stop complaints about unused functions
# CFLAGS += -Wno-unused-function

CFLAGS += -I.
CFLAGS += -g
CFLAGS += -Wall -Wextra -Wshadow
CFLAGS += -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_XOPEN_SOURCE

# CFLAGS += -std=c89 -Wno-gcc-compat -Wno-comment
# CFLAGS += -std=c99
CFLAGS += -std=c11

LIBRARY = lib$(NAME).a

all: $(LIBRARY)

C_SRC_LIB = \
	log.c \
	slice.c \
	buffer.c \
	utf8.c \
	ymd.c \
	hms.c \
	timer.c \
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
	cc $(CFLAGS) $(LDFLAGS) -o $@ $^ -ltap

tests: $(C_EXE_TEST)

test: tests
	@for t in $(C_EXE_TEST); do ./$$t; done

valgrind: tests
	@for t in $(C_EXE_TEST); do valgrind -q ./$$t; done

clean:
	rm -f *.o
	rm -f $(LIBRARY)
	rm -f $(C_OBJ_TEST) $(C_EXE_TEST)
