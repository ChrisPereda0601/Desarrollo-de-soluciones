CC = gcc
CFLAGS = -Wall -Wextra
TARGET = chat

all: $(TARGET)

$(TARGET): chat.o
	$(CC) $(CFLAGS) -o $(TARGET) chat.o

chat.o: chat.c
	$(CC) $(CFLAGS) -c chat.c

clean:
	rm -f $(TARGET) *.o