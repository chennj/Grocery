#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#define BUFFER_MAX 10
#define OK 0
#define ERROR -1

#define LEDBASE 0x56000010
#define LEDLEN 0x0c

int dev_major=232;
int dev_minor=0;

dev_t devnum;
unsigned int subdevnum = 1;

struct cdev* g_pDev;
struct file_operations* g_pFile;

char* buffer;

int hello_open(struct inode* p, struct file* f)
{
	printk(KERN_INFO"hello open\r\n");
	return 0;
}

ssize_t hello_write(struct file* f, const char __user* u, size_t s, loff_t* l)
{
	printk(KERN_INFO"hello write\r\n");
	return 0;
}

ssize_t hello_read(struct file* f, char __user* u, size_t s, loff_t* l)
{
	printk(KERN_INFO"hello read\r\n");
	return 0;
}

static int hellodev_probe(struct platform_device * pdev)
{	
	printk(KERN_INFO"hellodev probe\r\n");
	
	// 根据主次设备号生成设备号
	// devnum唯一的去标识一个设备
	// 主设备号（dev_major）：标识一类设备
	devnum = MKDEV(dev_major,dev_minor);
	// 将设备号注册到内核，register_chrdev_region标识注册的是字符设备
	// subdevnum：表示从 devnum 开始注册 subdevnum 个设备
	if (OK != register_chrdev_region(devnum,subdevnum,"hellodev")){
		printk(KERN_INFO"register hello dev failed\r\n");
		return ERROR;
	}
	
	printk(KERN_INFO"register hello dev success\r\n");
	
	// 分配内存
	g_pDev 	= kzalloc(sizeof(struct cdev), GFP_KERNEL);
	g_pFile = kzalloc(sizeof(struct file_operations), GFP_KERNEL);
	
	// 分别将我们的函数赋值给 file_operators 的相应的函数指针
	// 当我们在应用层对某个设备或者文件调用 open/read/write 函数的时候，
	// 它会跑到内核里面相应的 file_operations ，调用相应的 open/read/write
	// 函数
	g_pFile->open 	= hello_open;
	g_pFile->read 	= hello_read;
	g_pFile->write 	= hello_write;
	
	g_pFile->owner 	= THIS_MODULE;
	
	// 建立字符设备（g_pDev）与 file_operations 的联系
	cdev_init(g_pDev,g_pFile);
	// 建立字符设备（g_pDev）与 设备号的联系
	cdev_add(g_pDev,devnum,subdevnum);
	
	return 0;
}

static int hellodev_remove(struct platform_device * pdev)
{
	printk(KERN_INFO"hellodev remove\r\n");
	
	cdev_del(g_pDev);
	kfree(g_pFile);
	kfree(g_pDev);
	unregister_chrdev_region(devnum,subdevnum);
	
	return 1;
}

static void hello_plat_release(struct device* pdev)
{
	return;
}

static struct resource hello_dev_resource[] = {
	[0] = {
		.start = LEDBASE,
		.end = LEDBASE + LEDLEN - 1,
		.flags = IORESOURCE_MEM,
	}
};

static struct platform_device hello_device = {
	// 这个name 如果和 hellodev_driver.driver.name 如果不相同，
	// 则这个设备将匹配不到它的驱动，则 hellodev_probe 函数就不会
	// 执行。
	// 这个匹配过程在 platform_device_source.c 中有讲解
	// 在它的最后一行
	//.name = "hello-device",
	.name = "hello-dev",
	.id		= -1,
	.num_resources = ARRAY_SIZE(hello_dev_resource),
	.resource = hello_dev_resource,
	.dev = {
		.release = hello_plat_release,
	}
};

static struct platform_driver hellodev_driver = {
	.probe = hellodev_probe,
	.remove = hellodev_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hello-dev",
	},
};

int charDrvInit(void)
{
	printk(KERN_INFO"hellodev driver init\r\n");
	platform_device_register(&hello_device);
	return platform_driver_register(&hellodev_driver);
}

void __exit charDrvExit(void)
{
	printk(KERN_INFO"hellodev driver exit\r\n");
	platform_device_unregister(&hello_device);
	platform_driver_unregister(&hellodev_driver);
	return;
}

module_init(charDrvInit);		// 声明驱动程序的入口函数
module_exit(charDrvExit);		// 声明驱动程序的出口函数
MODULE_LICENSE("GPL");			// 版权

// 安装驱动 insmod helloDev.ko
// 卸载驱动 rmmod helloDev.ko
// 加载驱动 lsmod helloDev.ko