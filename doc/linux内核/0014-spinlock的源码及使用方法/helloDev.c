#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

#define BUFFER_MAX 64
#define OK 0
#define ERROR -1

int dev_major=232;
int dev_minor=0;

dev_t devnum;
unsigned int subdevnum = 1;

struct cdev* g_pDev;
struct file_operations* g_pFile;

char buffer[BUFFER_MAX];

//struct semaphore sema;
spinlock_t count_lock;
int open_count = 0;

int hello_open(struct inode* p, struct file* f)
{
	// 临界区
	// --------------------------------------------------------
	//down(&sema);		// 加锁
	spin_lock(&count_lock);
	if (open_count >= 1){
		//up(&sema);	// 失败后释放锁
		spin_unlock(&count_lock);
		printk(KERN_INFO"device is busy, hello_open fail\r\n");
		return -EBUSY;
	}
	open_count++;
	spin_unlock(&count_lock);
	//up(&sema);		// 成功后释放锁
	// --------------------------------------------------------
	
	printk(KERN_INFO"hello_open ok\r\n");
	return 0;
}

int hello_close(struct inode* p, struct file* f)
{
	if (open_count != 1){
		printk(KERN_INFO"something wrong, hello_close fail\r\n");
		return -EFAULT;
	}
	open_count--;
	printk(KERN_INFO"hello_close ok\r\n");
	return 0;
}

ssize_t hello_write(struct file* f, const char __user* u/*用户空间缓冲区的一个地址*/, size_t s, loff_t* l)
{
	printk(KERN_INFO"hello write\r\n");
	size_t writelen = 0;
	writelen = BUFFER_MAX > s ? s : BUFFER_MAX;
	// 将用户空间的数据拷贝到内核空间
	if (copy_from_user(buffer, u, writelen)){
		return -EFAULT;
	}
	return writelen;
}

ssize_t hello_read(struct file* f, char __user* u/*用户空间缓冲区的一个地址*/, size_t s, loff_t* l)
{
	printk(KERN_INFO"hello read\r\n");
	size_t readlen = 0;
	readlen = BUFFER_MAX > s ? s : BUFFER_MAX;
	// 将内核空间的数据拷贝到用户空间
	if (copy_to_user(u, buffer, readlen)){
		return -EFAULT;
	}
	return readlen;
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
		printk(KERN_INFO"register hello dev failed\r\n");
		return ERROR;
	}
	
	printk(KERN_INFO"register hello dev success\r\n");
	
	// 分配内存
	g_pDev 	= kzalloc(sizeof(struct cdev), GFP_KERNEL);
	g_pFile = kzalloc(sizeof(struct file_operations), GFP_KERNEL);
	
	// 分别将我们的函数赋值给 file_operations 的相应的函数指针
	// 当我们在应用层对某个设备或者文件调用 open/read/write 函数的时候，
	// 它会跑到内核里面相应的 file_operations ，调用相应的 open/read/write
	// 函数
	g_pFile->open 		= hello_open;
	g_pFile->release	= hello_close;
	g_pFile->read 		= hello_read;
	g_pFile->write 		= hello_write;
	
	g_pFile->owner 		= THIS_MODULE;
	
	// 建立字符设备（g_pDev）与 file_operations 的联系
	cdev_init(g_pDev,g_pFile);
	// 建立字符设备（g_pDev）与 设备号的联系
	cdev_add(g_pDev,devnum,subdevnum);
	
	// 初始化信号量
	//sema_init(&sema, 1);
	// 初始化自旋锁
	spin_lock_init(&count_lock);
	
	return 0;
}

void __exit hello_exit(void)
{
	printk(KERN_INFO"unregister helloDev\r\n");
	
	cdev_del(g_pDev);
	kfree(g_pFile);
	kfree(g_pDev);
	unregister_chrdev_region(devnum,subdevnum);
	
	return ;
}

module_init(hello_init);		// 声明驱动程序的入口函数
module_exit(hello_exit);		// 声明驱动程序的出口函数
MODULE_LICENSE("GPL");			// 版权

// 安装驱动 insmod
// 卸载驱动 rmmod
// 加载驱动 lsmod