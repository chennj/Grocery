DEPEND		= ../../depends
SRC 		= $(DEPEND)/src
INCLUDE 	= -I $(DEPEND)/include/net -I $(DEPEND)/thrd-json -I $(DEPEND)/thrd-sha

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= nlcontrol
SERVERSRC 	= control_main.cpp control.cpp \
			$(SRC)/net/*.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp

all: $(SERVER)
$(SERVER):
	$(CXX) -g -c -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS);
	$(CXX) -g -o $(SERVER) *.o $(LIBS);

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.txt
	rm -rf nlcontrol 
