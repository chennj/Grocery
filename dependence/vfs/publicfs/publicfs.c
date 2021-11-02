#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/slab.h>

/*
 *  一个文件系统的三个基本结构及其操作：
 *  -----------------------------------------
 *  super_block             每个文件夹都有一个这样的超级块
 *  super_block_operation
 *  inode                   存储文件数据
 *  inode_operations
 *  file                    存储文件属性
 *  file_operation
 *  mount 命令用来挂载文件系统。其基本命令格式为：
 *  -----------------------------------------
 *  mount -t type [-o options] device dir
 *  device：指定要挂载的设备，比如磁盘、光驱等。
 *  dir：指定把文件系统挂载到哪个目录。
 *  type：指定挂载的文件系统类型，一般不用指定，mount 命令能够自行判断。
 *  options：指定挂载参数，比如 ro 表示以只读方式挂载文件系统。
 **/

#define BLOCK_SIZE  1024

unsgined char block[BLOCK_SIZE];

ssize_t publicfs_file_write (struct file *, char __user * buffer, size_t length, loff_t *)
{
    copy_from_user(buffer, block, length);
}

ssize_t publicfs_file_read (struct file *, const char __user * buffer, size_t length, loff_t *)
{
    copy_to_user(block, buffer, length);
}

struct file_operations publicfs_inode_fops = 
{
    .write = publicfs_file_write,
    .read = publicfs_file_read
};

//创建一个文件 touch xxx
int publicfs_create (struct inode * dir,struct dentry * dentry_, umode_t umode_t_, bool excl)
{
    struct super_block * sb = dir->i_sb;

    struct inode * inode = new_inode(sb);

    //填写文件根节点
    inode->i_sb = sb;
    //文件操作
    //栗子：
    //写数据到文件 echo 1 > a.txt，调用的就是 file_operations.write
    //从文件读数据 cat a.txt，调用的就是 file_operations.read
    inode->i_fop = &publicfs_inode_fops;
    //分配文件数据存储位置 
    //比如写数据 echo 1 > a.txt，数据最终写入到 i_private中
    inode->i_private = block;
}

int publicfs_mkdir (struct inode * inode_,struct dentry * dentry_, umode_t umode_t_)
{

}

int publicfs_rmdir(struct inode *,struct dentry *)
{

}

static struct inode_operations publicfs_inode_ops = 
{
    .create = publicfs_create,
    .mkdir = publicfs_mkdir,
    .rmdir = publicfs_rmdir
};

//填充超级块 super_block
int publicfs_fill_super(struct super_block * sb, void * data, int silent)
{
    struct inode * root_node = new_inode(sb);

    root_node->i_sb = sb;
    root_node->i_op = &publicfs_inode_ops;
}

struct dentry * publicfs_mount(struct file_system_type * fs_type, int flags, const char * devname, void * data)
{
    // nodev 表示不带磁盘（存储）的 dev
    // 带磁盘与不带磁盘的区别：nodev表示是一个内存文件系统，类似 /proc /sys
    return mount_nodev(fs_type, flags, data);
}

void publicfs_kill_sb (struct super_block * sb)
{

}

struct file_system_type publicfs_type = 
{
    .owner      = THIS_MODULE,
    .name       = "publicfs",           // 在console中使用 (u)mount -t publicfs nodev /mnt/中所使用的名字
    .mount      = publicfs_mount,       // 在挂载（mount） 本文件系统时调用：mount -t publicfs nodev /mnt/
    .kill_sb    = publicfs_kill_sb,     // 在注销（umount）本文件系统时调用：umount /mnt/
};


//这个函数在insmod publicfs.ko的时候执行
static int publicfs_init(void)
{
    int ret = register_filesystem(&publicfs_type);
    if (ret){
        printk("register publicfs failed.\n");
    }
    
    return 
}

module_init(publicfs_init);