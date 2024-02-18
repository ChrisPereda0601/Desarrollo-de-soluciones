CC ?= gcc
CXX ?= g++
CPP ?= g++

APP_NAME = i2c_test
APP_OBJ_FILES = i2c_test.o
APP_NAME_SERVER = i2c_server
APP_OBJ_FILES_SERVER = i2c_server.o
APP_NAME_CLIENT = i2c_client
APP_OBJ_FILES_CLIENT = i2c_client.o

LIBS = 

all: $(APP_NAME) $(APP_NAME_SERVER) $(APP_NAME_CLIENT)

$(APP_NAME) : $(APP_OBJ_FILES)
	$(CXX) $^ -o $@  $(LIBS)

$(APP_NAME_SERVER) : $(APP_OBJ_FILES_SERVER)
	$(CXX) $^ -o $@  $(LIBS)

$(APP_NAME_CLIENT) : $(APP_OBJ_FILES_CLIENT)
	$(CXX) $^ -o $@  $(LIBS)

%.o : %.cc
	$(CXX) -c $^ -o $@


clean:
	rm *.o $(APP_NAME) $(APP_NAME_SERVER) $(APP_NAME_CLIENT)