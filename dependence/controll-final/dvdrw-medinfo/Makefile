DEST=libdvd+rw.a   
ALL:$(DEST) 
	 ar  crv libdvd+rw.a dvd+rw-mediainfo.o 
$(DEST):dvd+rw-mediainfo.o 
  
dvd+rw-mediainfo.o:dvd+rw-mediainfo.cpp
	 g++ -g -O2 -fno-exceptions -D_REENTRANT   -c -o dvd+rw-mediainfo.o dvd+rw-mediainfo.cpp  -I.
main:libdvd+rw.a main.c
	gcc main.c -g  -o main  -I.  -L. -lblkid -ldvd+rw -lstdc++ 
clean:
	-rm *.a
	-rm *.o
	-rm main
