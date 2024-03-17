CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS =

SRC = botones.c
OBJ = $(SRC:.c=.o)
TARGET = botones

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
