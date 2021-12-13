SRC 		= ../../../depends/src
INCLUDE 	= -I ../../include -I../../../depends/include/net

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= mserver
SERVERSRC	= svr_machine_main.cpp \
			svr_machine_server.cpp \
			svr_machine_client.cpp \
			$(SRC)/net/crc_easy_select_server.cpp \
			$(SRC)/net/crc_easy_tcp_server.cpp \
			$(SRC)/net/crc_easy_select_client.cpp \
			$(SRC)/net/crc_easy_tcp_client.cpp \
			$(SRC)/net/crc_buffer.cpp \
			$(SRC)/net/crc_client.cpp \
			$(SRC)/net/crc_fdset.cpp \
			$(SRC)/net/crc_semaphore.cpp \
			$(SRC)/net/crc_task.cpp \
			$(SRC)/net/crc_thread.cpp \
			$(SRC)/net/crc_work_select_server.cpp \
			$(SRC)/net/crc_work_server.cpp 


all: $(SERVER)
mserver:
	$(CXX) -g -o mserver -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf mserver
