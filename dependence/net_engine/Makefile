SRC 	= src
INCLUDE = -I include

CXX		= g++
LIBS	= -lpthread
FLAGS	= -Wall

TARGET	= server

SOURCES = $(SRC)/server/server.cpp \
          $(SRC)/core/crc_boss_epoll_server.cpp \
          $(SRC)/core/crc_boss_server.cpp \
          $(SRC)/core/crc_buffer.cpp \
		  $(SRC)/core/crc_channel.cpp \
		  $(SRC)/core/crc_config.cpp \
		  $(SRC)/core/crc_epoll.cpp \
		  $(SRC)/core/crc_net_work.cpp \
		  $(SRC)/core/crc_semaphore.cpp \
		  $(SRC)/core/crc_stream.cpp \
		  $(SRC)/core/crc_stream_message.cpp \
		  $(SRC)/core/crc_task_server.cpp \
		  $(SRC)/core/crc_thread.cpp \
		  $(SRC)/core/crc_work_epoll_server.cpp \
		  $(SRC)/core/crc_work_server.cpp 

all: $(TARGET)
server:
	$(CXX) -o server -std=c++11 $(SOURCES) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf server 