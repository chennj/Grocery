#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#define DATA_NUM 32

int main(int argc, char* argv)
{
	int fd, i;
	int w_len;
	fd_set fdset;
	char buf[DATA_NUM] = "hello linux";

	fd = open("/dev/hello", O_RDWR);
	printf("%d\r\n",fd);
	if (-1 == fd){
		perror("open file error\r\n");
		return -1;
	}

	printf("open success\r\n");

	w_len = write(fd, buf, DATA_NUM);

	if (-1 == w_len){
		perror("write error\r\n");
		return -1;
	}
	
	sleep(5);
	
	printf("write len: %d\r\n", w_len);
	close(fd);
	
	return 0;

}

 