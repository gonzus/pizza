NAME = pizza

# check memory allocations
# CFLAGS += -DMEM_CHECK=1

# use realloc instead of malloc / free
# CFLAGS += -DMEM_USE_REALLOC_INSTEAD_OF_MALLOC_AND_FREE=1

# see more log messages
CFLAGS += -DLOG_LEVEL=1

# uncomment to make warnings into errors
# CFLAGS += -Werror

# uncomment to stop complaints about unused functions
# CFLAGS += -Wno-unused-function

CFLAGS += -I.
CFLAGS += -g
CFLAGS += -Wall -Wextra

# CFLAGS += -std=c89 -Wno-gcc-compat -Wno-comment
# CFLAGS += -std=c99
CFLAGS += -std=c11

all: main

LIBRARY = lib$(NAME).a

C_SRC_LIB = \
	log.c \
	mem.c \
	slice.c \
	buffer.c \
	utf8.c \
	date.c \

C_OBJ_LIB = $(C_SRC_LIB:.c=.o)

$(LIBRARY): $(C_OBJ_LIB)
	ar -crs $@ $^

%.o: %.c
	cc $(CFLAGS) -c -o $@ $^

main: main.o $(LIBRARY)
	cc $(CFLAGS) $(LDFLAGS) -o $@ $^

clean:
	rm -f *.o
	rm -f $(LIBRARY)
	rm -f main
