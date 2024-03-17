CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME_CAPTURE = client
OBJFILES_CAPTURE = client.o

APP_NAME_PLAY = server
OBJFILES_PLAY = server.o


LIB_DIRS = .
LIBS = -lasound

all: $(APP_NAME_CAPTURE) $(APP_NAME_PLAY)  $(APP_NAME_VOL)

$(APP_NAME_CAPTURE): $(OBJFILES_CAPTURE)
	$(CC) $^ -o $@ -L$(LIB_DIRS) $(LIBS)

$(APP_NAME_PLAY): $(OBJFILES_PLAY)
	$(CC) $^ -o $@ -L$(LIB_DIRS) $(LIBS)


%.o: %.c
	$(CC) -c $^ -o $@

clean:
	rm *.o  $(APP_NAME_CAPTURE) $(APP_NAME_PLAY)

fresh:
	make clean
	make all 