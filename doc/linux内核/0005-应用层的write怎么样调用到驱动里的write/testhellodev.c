 #include <fcntl>
 #include <stdio.h>
 #include <string.h>
 #include <sys/select.h>
 
 #define DATA_NUM 64
 int main(int argc, char* argv)
 {
	 int fd, i;
	 int r_len, w_len;
	 fd_set fdset;
	 char buf[DATA_NUM] = "hello dev";
	 
	 memset(buf, 0, DATA_NUM);
	 
	 fd = open("/dev/hello", O_RDWR);
	 printf("%d\r\n",fd);
	 if (-1 == fd){
		 perror("open file error\r\n");
		 return -1;
	 }
	 
	 printf("open success\r\n");
	 
	 w_len = write(fd, buf, DATA_NUM);
	 r_len = read(fd, buf, DATA_NUM);
	 
	 printf("%d %d\r\n", w_len, r_len);
	 printf("%s\r\n", buf);
	 return 0;
 }
 
 // 1. 	编译 
 //		$>gcc -o test testhellodev.c
 // 2. 	创建字符设备文件 
 //		$>mknod /dev/hello c 232 0
 // 	其中232 0 对应的是 0004里面的hellodev.c里面的 dev_major dev_minor
 // 3.	查看一下我们建立的字符设备文件 
 //		$>ls -l /dev/hello
 // 4.	清理一下内核日志 
 //		$>dmesg -c
 // 5.	执行
 //		./testhellodev