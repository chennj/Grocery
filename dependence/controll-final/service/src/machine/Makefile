#不能使用静态编译，否则无法运行
#警告： Using 'getaddrinfo' in statically linked applications requires at runtime the shared libraries from the glibc version used for linking

SRC 		= ../../../depends/src
DEPEND		= ../../../depends
INCLUDE 	= \
			-I ../../include \
			-I ../../../depends/include/net \
			-I ../../../depends/thrd-json \
			-I ../../../depends/thrd-sha \
			-I ../../../dvdrw-medinfo/ \
			-I ../../../depends/threadpoll

STATIC		= 
CXX			= g++
LIBS		= -lpthread -L../../../dvdrw-medinfo/ -lblkid -ldvd+rw
FLAGS		= -Wall

SERVER		= machineserver
SERVERSRC 	= \
			svr_machine_main.cpp svr_machine.cpp \
			$(SRC)/net/*.cpp \
			$(DEPEND)/threadpool/crc_thread_pool.cpp \
			$(DEPEND)/thrd-json/CJsonObject.cpp \
			$(DEPEND)/thrd-json/cJSON.c \
			$(DEPEND)/thrd-sha/sha1.cpp

all: $(SERVER)
machineserver:
	$(CXX) -g -o machineserver -std=c++11 $(SERVERSRC) $(INCLUDE) ../../../dvdrw-medinfo/libdvd+rw.a $(LIBS) $(FLAGS) $(STATIC)

.PHONY: clean
clean:
	rm -rf *.txt
	rm -rf machineserver 