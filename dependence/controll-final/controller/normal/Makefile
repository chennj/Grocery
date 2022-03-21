DEPEND		= ../../depends
SRC 		= $(DEPEND)/src
INCLUDE 	= \
			-I . \
			-I $(DEPEND)/include/net \
			-I $(DEPEND)/include/db \
			-I $(DEPEND)/thrd-json \
			-I $(DEPEND)/thrd-sha \
			-I $(DEPEND)/thrd-sqlite \
			-I $(DEPEND)/threadpool

CXX			= g++
CC			= gcc
LIBS		= -lpthread -ldl
FLAGS		= -Wall

SERVER		= nlcontroller
SERVERSRC 	= controller_main.cpp controller.cpp \
			$(SRC)/net/*.cpp \
			$(SRC)/db/*.cpp \
			$(DEPEND)/threadpool/crc_thread_pool.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp \
			sqlite3.o CppSQLite3.o

all: $(SERVER)
$(SERVER):
	$(CC)  -c $(DEPEND)/thrd-sqlite/sqlite3.c
	$(CC)  -c $(DEPEND)/thrd-sqlite/shell.c
	$(CXX) -c $(DEPEND)/thrd-sqlite/CppSQLite3.cpp

	$(CXX) -g -o $(SERVER) -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS)

.PHONY: clean
clean:
	rm -rf *.o
	rm -rf *.txt
	rm -rf $(SERVER) 
