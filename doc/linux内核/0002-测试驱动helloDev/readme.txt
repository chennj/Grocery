本节来看驱动的测试。

我们需要编写一个应用层的程序来对hello驱动进行测试：(test.c)

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>


#define DATA_NUM    (64)
int main(int argc, char *argv[])
{
   int fd, i;
   int r_len, w_len;
   fd_set fdset;
   char buf[DATA_NUM]="hello world";
   memset(buf,0,DATA_NUM);
   fd = open("/dev/hello", O_RDWR);
printf("%d\r\n",fd);
   if(-1 == fd) {
      perror("open file error\r\n");
return -1;
   }
else {
printf("open successe\r\n");
}
   
   w_len = write(fd,buf, DATA_NUM);
   r_len = read(fd, buf, DATA_NUM);
   printf("%d %d\r\n", w_len, r_len);
   printf("%s\r\n",buf);

   return 0;
}

编译并执行，发现错误，找不到设备文件：


这是因为还没有创建hello驱动的设备文件，我们为hello驱动手动创建设备文件：

root@ubuntu:/home/jinxin/drivers/helloDev# mknod /dev/hello c 232 0

备注：这里的232和0要跟驱动文件里定义的主次设备号对应起来！

然后再次执行测试程序，发现成功了：

root@ubuntu:/home/jinxin/drivers/helloDev# ./test
3
open successe
0 0

root@ubuntu:/home/jinxin/drivers/helloDev#

然后再次执行dmesg查看驱动输出，发现驱动里的hell_open, hello_write, hello_read被依次调用了。


这就是一个完整的、最简单的驱动的开发和测试的流程。

我想大家可能会有几个问题：

1.驱动测试的时候为什么要有设备文件，设备文件的作用是什么？hello驱动的设备文件创建的时候为什么要指定主设备号为232, 此设备号为0？

2.对/dev/hello执行write()调用的时候，怎么就调用到了驱动里的hello_write()里去了？

3.测试程序的read和write的返回值为什么都是0？ 作者：简说linux https://www.bilibili.com/read/cv7241587 出处：bilibili