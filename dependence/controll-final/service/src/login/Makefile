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

SERVER		= loginserver
SERVERSRC 	= \
			svr_login_main.cpp svr_login.cpp \
			$(SRC)/net/*.cpp \
			$(SRC)/db/*.cpp \
			$(DEPEND)/threadpool/crc_thread_pool.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp \
			sqlite3.o CppSQLite3.o

all: $(SERVER)
loginserver:
	$(CC)  -c $(DEPEND)/thrd-sqlite/sqlite3.c
	$(CC)  -c $(DEPEND)/thrd-sqlite/shell.c
	$(CXX) -c $(DEPEND)/thrd-sqlite/CppSQLite3.cpp

	$(CXX) -g -o loginserver -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS)

.PHONY: clean
clean:
	rm -rf loginserver 
	rm -rf *.txt
	rm -rf *.o
	rm -rf *.db
