SRC 		= ../../depends/src
INCLUDE 	= -I ../include -I ../../depends/include/net

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= vstest
SERVERSRC	= virtual_server_test.cpp \
			virtual_server.cpp \
			$(SRC)/net/crc_easy_select_server.cpp \
			$(SRC)/net/crc_easy_tcp_server.cpp \
			$(SRC)/net/crc_buffer.cpp \
			$(SRC)/net/crc_client.cpp \
			$(SRC)/net/crc_fdset.cpp \
			$(SRC)/net/crc_semaphore.cpp \
			$(SRC)/net/crc_task.cpp \
			$(SRC)/net/crc_thread.cpp \
			$(SRC)/net/crc_work_select_server.cpp \
			$(SRC)/net/crc_work_server.cpp 

all: $(SERVER)
vstest:
	$(CXX) -o vstest -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf vstest
