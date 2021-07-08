#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define OK 0
#define ERROR -1

int dev_major=232
int dev_minor=0

dev_t devnum;
unsigned int subdevnum = 1;

struct cdev* g_pDev;
struct file_operators* g_pFile;

int hello_open(struct inode* p, struct file* f)
{
	printk("hello open\r\n");
	return 0;
}

ssize_t hello_write(struct file* f, const char __user* u, size_t s, loff_t* l)
{
	printk(KERN_EMERG"hello write\r\n");
	return 0;
}

ssize_t hello_read(struct file* f, char __user* u, size_t s, loff_t* l)
{
	printk(KERN_EMERG"hello read\r\n");
	return 0;
}

int hello_init(void)
{	
	// 根据主次设备号生成设备号
	// devnum唯一的去标识一个设备
	// 主设备号（dev_major）：标识一类设备
	devnum = MKDEV(dev_major,dev_minor);
	// 将设备号注册到内核，register_chrdev_region标识注册的是字符设备
	// subdevnum：表示从 devnum 开始注册 subdevnum 个设备
	if (OK != register_chrdev_region(devnum,subdevnum,"hellodev")){
		printk(KERN_EMERG"register hello dev failed\r\n");
		return ERROR;
	}
	
	printk(KERN_EMERG"register hello dev success\r\n");
	
	// 分配内存
	g_pDev 	= kzalloc(sizeof(struct cdev), GFP_KERNEL);
	g_pFile = kzalloc(sizeof(struct file_operators), GFP_KERNEL);
	
	// 分别将我们的函数赋值给 file_operators 的相应的函数指针
	// 当我们在应用层对某个设备或者文件调用 open/read/write 函数的时候，
	// 它会跑到内核里面相应的 file_operators ，调用相应的 open/read/write
	// 函数
	g_pFile->open = hello_open;
	g_pFile->read = hello_read;
	g_pFile->write = hello_write;
	
	g_pFile->owner = THIS_MUDOLE;
	
	// 建立字符设备（g_pDev）与 file_operators 的联系
	cdev_init(g_pDev,g_pFile);
	// 建立字符设备（g_pDev）与 设备号的联系
	cdev_add(g_pDev,devnum,subdevnum);
	return 0;
}

void __exit hello_exit(void)
{
	cdev_del(p_pDev);
	unregister_chrdev_region(devnum,subdevnum);
	
	printk(KERN_EMERG"unregister hello dev\r\n");
	
	return ;
}

module_init(hello_init);		// 声明驱动程序的入口函数
module_exit(hello_exit);		// 声明驱动程序的出口函数
MODULE_LECENCE("GPL");			// 版权

// 安装驱动 insmod helloDev.ko
// 卸载驱动 rmmod helloDev.ko
// 加载驱动 lsmod helloDev.ko