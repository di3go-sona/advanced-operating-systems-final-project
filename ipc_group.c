#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>
#include <linux/list.h>
#include "ipc_module_costants.h"






int ipc_group_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "ipc dev open");

	return 0;
}


ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {
    printk(KERN_INFO "ipc dev read, buf: %p, len: %lu\n", buf, lrn);

	return 0;
}



int ipc_group_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "ipc dev release");

	return 0;
}

struct file_operations ipc_group_ops = {
	.open = ipc_group_open,
	.read = ipc_group_read,
	.release = ipc_group_release
};


