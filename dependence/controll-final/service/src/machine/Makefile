SRC 		= ../../../depends/src
DEPEND		= ../../../depends
INCLUDE 	= -I ../../include -I ../../../depends/include/net -I ../../../depends/thrd-json -I ../../../depends/thrd-sha

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= machineserver
SERVERSRC 	= \
			svr_machine_main.cpp svr_machine.cpp \
			$(SRC)/net/*.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp

all: $(SERVER)
machineserver:
	$(CXX) -g -o machineserver -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf *.txt
	rm -rf machineserver 