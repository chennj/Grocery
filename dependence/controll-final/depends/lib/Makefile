SRC 		= ../src
THIRD		= ..
INCLUDE 	= -I../include/net -I../thrd-json -I../thrd-sha

CXX			= g++
LIBS		= -lpthread
FLAGS		= -Wall

LIB_NAME	= crcnet
STATIC_NAME = lib$(LIB_NAME).a
SHARE_NAME  = lib$(LIB_NAME).so

LIBSRC 		= \
			$(SRC)/net/*.cpp \
			$(THIRD)/thrd-json/CJsonObject.cpp \
			$(THIRD)/thrd-json/cJSON.c \
			$(THIRD)/thrd-sha/sha1.cpp

all: static_library shared_library
static_library: 
	$(CXX) -g -c -std=c++11 $(LIBSRC) $(INCLUDE) $(LIBS) $(FLAGS);
	ar -crv $(STATIC_NAME) *.o;
 
shared_library:
	$(CXX) -shared -fpic -g -o $(SHARE_NAME) $(LIBSRC) $(INCLUDE) $(LIBS) $(FLAGS);
 
install:
	mv $(STATIC_NAME) lib
	mv $(SHARE_NAME) lib
 
clean:
	rm -rf *.o
	rm -rf $(STATIC_NAME) $(SHARE_NAME)
