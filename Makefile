CFLAGS += -g

all: main

pizza.o: pizza.c
main.o: main.c

main: main.o pizza.o

clean:
	rm -f *.o
	rm -f main
