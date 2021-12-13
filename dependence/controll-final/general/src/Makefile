SRC 		= ../../depends/src
DEPEND		= ../../depends
INCLUDE 	= -I ../include -I ../../depends/include/net -I ../../depends/thrd-json -I ../../depends/thrd-sha

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

SERVER		= controller
SERVERSRC 	= gen_controller_main.cpp gen_controller.cpp \
			$(SRC)/net/crc_net_client_s.cpp \
			$(SRC)/net/crc_net_server.cpp \
			$(SRC)/net/crc_net_transfer.cpp \
			$(SRC)/net/crc_easy_epoll_client.cpp \
			$(SRC)/net/crc_easy_tcp_client.cpp \
			$(SRC)/net/crc_easy_epoll_server.cpp \
			$(SRC)/net/crc_easy_tcp_server.cpp \
			$(SRC)/net/crc_client_s_http.cpp \
			$(SRC)/net/crc_client_s_websocket.cpp \
			$(SRC)/net/crc_easy_websocket_server.cpp \
			$(SRC)/net/crc_work_server.cpp \
			$(SRC)/net/crc_work_epoll_server.cpp \
			$(SRC)/net/crc_buffer.cpp \
			$(SRC)/net/crc_client.cpp \
			$(SRC)/net/crc_epoll.cpp \
			$(SRC)/net/crc_semaphore.cpp \
			$(SRC)/net/crc_task.cpp \
			$(SRC)/net/crc_thread.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp

all: $(SERVER)
controller:
	$(CXX) -o controller -std=c++11 $(SERVERSRC) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf controller 
