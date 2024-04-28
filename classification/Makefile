CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME = tflite_classification_example
APP_OBJ_FILES = tflite_classification_example.o

LIBS = -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio -lopencv_imgcodecs -ltensorflow-lite

all: $(APP_NAME)

$(APP_NAME) : $(APP_OBJ_FILES)
	$(CXX) $^ -o $@  $(LIBS)

%.o : %.cc
	$(CXX) -c $^ -o $@


clean:
	rm *.o $(APP_NAME)