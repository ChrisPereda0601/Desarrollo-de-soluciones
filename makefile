CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME = hello_server
OBJFILES = hello_server.o

APP_NAME_CLIENT = hello_client
OBJFILES_CLIENT = hello_client.o

APP_NAME_UNIX_SERVER = hello_server_unix
OBJFILES_UNIX_SERVER = hello_server_unix.o

APP_NAME_UNIX_CLIENT = hello_client_unix
OBJFILES_UNIX_CLIENT = hello_client_unix.o


LIB_DIRS = .

all: $(APP_NAME) $(APP_NAME_CLIENT) $(APP_NAME_UNIX_SERVER) $(APP_NAME_UNIX_CLIENT)

$(APP_NAME): $(OBJFILES)
	$(CC) $^ -o $@ -L$(LIB_DIRS)

$(APP_NAME_CLIENT): $(OBJFILES_CLIENT)
	$(CC) $^ -o $@ -L$(LIB_DIRS)

$(APP_NAME_UNIX_SERVER): $(OBJFILES_UNIX_SERVER)
	$(CC) $^ -o $@ -L$(LIB_DIRS)

$(APP_NAME_UNIX_CLIENT): $(OBJFILES_UNIX_CLIENT)
	$(CC) $^ -o $@ -L$(LIB_DIRS)

%.o: %.c
	$(CC) -c $^ -o $@

clean:
	rm *.o  $(APP_NAME) $(APP_NAME_CLIENT) $(APP_NAME_UNIX_SERVER) $(APP_NAME_UNIX_CLIENT)

fresh:
	make clean
	make all 