CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME_CAPTURE = audio_botones
OBJFILES_CAPTURE = audio_botones.o

APP_NAME_PLAY = audio_final
OBJFILES_PLAY = audio_final.o


APP_NAME_VOL = volume_control
OBJFILES_VOL = volume_control.o

LIB_DIRS = .
LIBS = -lasound

all: $(APP_NAME_CAPTURE) $(APP_NAME_PLAY)  $(APP_NAME_VOL)

$(APP_NAME_CAPTURE): $(OBJFILES_CAPTURE)
	$(CC) $^ -o $@ -L$(LIB_DIRS) $(LIBS)


$(APP_NAME_VOL): $(OBJFILES_VOL)
	$(CC) $^ -o $@ -L$(LIB_DIRS) $(LIBS)


$(APP_NAME_PLAY): $(OBJFILES_PLAY)
	$(CC) $^ -o $@ -L$(LIB_DIRS) $(LIBS)


%.o: %.c
	$(CC) -c $^ -o $@

clean:
	rm *.o  $(APP_NAME_CAPTURE) $(APP_NAME_PLAY) $(APP_NAME_VOL)

fresh:
	make clean
	make all 