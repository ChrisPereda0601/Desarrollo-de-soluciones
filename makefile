CC=gcc
CFLAGS=-Wall

all: chat2

chat2: chat2.o
	$(CC) $(CFLAGS) -o chat2 chat2.o

chat2.o: chat2.c
	$(CC) $(CFLAGS) -c chat2.c

clean:
	rm -f *.o chat2
