INCLUDE 	= \
			-I ../include \
			-I ../include/net \
			-I ../thrd-json \
			-I ../thrd-sha 
			
CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= threadpooltest
SERVERSRC 	= *.cpp \
			../src/net/*.cpp \
			../thrd-json/CJsonObject.cpp \
			../thrd-json/cJSON.c \
			../thrd-sha/sha1.cpp

all: $(SERVER)
threadpooltest:
	$(CXX) -g -o $(SERVER) -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.txt
	rm -rf threadpooltest 