CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME = proyecto
APP_OBJ_FILES = proyecto.o

LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs -lncurses

all: $(APP_NAME)

$(APP_NAME) : $(APP_OBJ_FILES)
	$(CXX) $^ -o $@ $(LIBS)

%.o : %.cc
	$(CXX) -c $^ -o $@

clean:
	rm -f *.o $(APP_NAME)
