SRC 		= ../../depends/src
DEPEND		= ../../depends
INCLUDE 	= -I ../include -I ../../depends/include/net -I ../../depends/thrd-json -I ../../depends/thrd-sha

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= controller
SERVERSRC 	= gen_controller_main.cpp gen_controller.cpp \
			$(SRC)/net/*.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp

all: $(SERVER)
controller:
	$(CXX) -g -c -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS);
	$(CXX) -g -o controller *.o $(LIBS);

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.txt
	rm -rf controller 
