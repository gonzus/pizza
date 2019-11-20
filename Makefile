# uncomment to see logging messages
# CFLAGS += -DLOG_LEVEL_DEFAULT=1

# uncomment to make warnings into errors
# CFLAGS += -Werror

CFLAGS += -I.
CFLAGS += -g
CFLAGS += -Wall -Wextra
CFLAGS += -Wno-unused-function

# CFLAGS += -std=c89 -Wno-gcc-compat -Wno-comment
# CFLAGS += -std=c99
CFLAGS += -std=c11

all: main

log.o: log.c
slice.o: slice.c
buffer.o: buffer.c
utf8.o: utf8.c
main.o: main.c

main: main.o log.o slice.o buffer.o utf8.o

clean:
	rm -f *.o
	rm -f main
