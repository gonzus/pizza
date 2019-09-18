# uncomment to see logging messages
# CFLAGS += -DLOG_LEVEL_DEFAULT=1

# uncomment to make warnings into errors
# CFLAGS += -Werror

CFLAGS += -I.
CFLAGS += -std=c99
# CFLAGS += -std=c11
CFLAGS += -g
CFLAGS += -Wall -Wextra

all: main

log.o: log.c
slice.o: slice.c
buffer.o: buffer.c
main.o: main.c

main: main.o log.o slice.o buffer.o

clean:
	rm -f *.o
	rm -f main
