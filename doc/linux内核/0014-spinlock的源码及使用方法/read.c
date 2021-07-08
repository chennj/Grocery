#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#define DATA_NUM 32

int main(int argc, char* argv)
{
	int fd, i;
	int r_len;
	fd_set fdset;
	char buf[DATA_NUM] = "hello dev";
	
	fd = open("/dev/hello", O_RDWR);

	printf("%d\r\n",fd);

	if (-1 == fd){
		perror("open file error\n");
		return -1;
	}

	printf("open success\n");

	r_len = read(fd, buf, DATA_NUM);
	if (-1 == r_len){
		perror("read error\n");
		return -1;
	}

	printf("read len: %d\n", r_len);
	printf("%s\n", buf);
	close(fd);

	return 0;

}

 