SRC 		= ../../../depends/src
DEPEND		= ../../../depends
INCLUDE 	= \
			-I ../../include \
			-I ../../../depends/include/net \
			-I ../../../depends/include/db \
			-I ../../../depends/thrd-json \
			-I ../../../depends/thrd-sha \
			-I ../../../depends/thrd-sqlite \
			-I ../../../depends/threadpoll

CXX			= g++
CC			= gcc
LIBS		= -lpthread -ldl
FLAGS		= -Wall -fpermissive

SERVER		= mkisoserver
SERVERSRC 	= \
			svr_mkiso_main.cpp svr_mkiso.cpp \
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
	rm -rf $(SERVER) 
	rm -rf *.txt
	rm -rf *.o
