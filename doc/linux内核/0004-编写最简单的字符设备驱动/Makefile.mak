ifneq($(KERNELRELEASE),)
obj-m 	:= helloDev.o
else
PWD 	:= $(shell pwd)
#KDIR	:= /home/njchen/linux-4.9.229
#KDIR	:= /lib/modules/4.4.0-31-generic/build
KDIR 	:= /lib/modules/`uname -r`/build
all:
	make -C $(KDIR) M=$(PWD)	#表示先进入KDIR目录执行本makefile文件，然后再回到PWD再执行一遍本makefile文件
								#即本 makefile 文件会执行两遍，第一次执行走的是 else 分支，第二次执行走的是 ifneq 分支。
clean:
	rm -rf *.o *.mod.c *.symvers *.order *.c~ *~
endif