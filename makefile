CC = gcc
CFLAGS = -Wall -Wextra
TARGET = chat2

all: $(TARGET)

$(TARGET): chat2.o
	$(CC) $(CFLAGS) -o $(TARGET) chat2.o

chat2.o: chat2.c
	$(CC) $(CFLAGS) -c chat2.c

clean:
	rm -f $(TARGET) *.o