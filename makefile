CC ?= gcc
 
APP_NAME = led3
OBJS = led3.o
 
all: $(APP_NAME)
 
$(APP_NAME): $(OBJS)
	$(CC) -o $@ $^
 
%.o: %.c
	$(CC) -c $^ -o $@
 
 
clean:
	rm *.o $(APP_NAME)