CFLAGS += -std=c99
CFLAGS += -I.
CFLAGS += -g
CFLAGS += -Wall -Wextra
# CFLAGS += -Werror

all: main

slice.o: slice.c
buffer.o: buffer.c
main.o: main.c

main: main.o slice.o buffer.o

clean:
	rm -f *.o
	rm -f main
