SRC 	= ../../src
INCLUDE = -I ../../include

CXX		= g++
LIBS	= -lpthread -lmysqlclient
FLAGS	= -Wall

TARGET	= demo

SOURCES = $(SRC)/mysql/demo.cpp \
          $(SRC)/mysql/crc_mysql_conn.cpp \
          $(SRC)/mysql/crc_mysql_pool_conn.cpp \
          $(SRC)/mysql/crc_mysql_preparestatement.cpp \
		  $(SRC)/mysql/crc_mysql_resultset.cpp \
		  $(SRC)/core/crc_task_server.cpp \
		  $(SRC)/core/crc_thread.cpp \
		  $(SRC)/core/crc_semaphore.cpp 

all: $(TARGET)
demo:
	$(CXX) -o demo -std=c++11 $(SOURCES) $(INCLUDE) $(LIBS) $(FLAGS) 

.PHONY: clean
clean:
	rm -rf server 