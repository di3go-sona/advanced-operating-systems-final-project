#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>

#include "ipc_module_costants.h"


extern struct class*  	dev_class ;

struct cdev* 	group_cdevs[IPC_MAX_GROUPS];


int ipc_group_open(struct inode *inode, struct file *filp) {
	
    printk(KERN_INFO "ipc dev open");

	return 0;
}
ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {

    printk(KERN_INFO "ipc dev read");

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




int ipc_group_install(group_t groupno, int dev_major)
{
	int res;

	
	dev_t devno;
	char devname[16] = {0};
	struct device*	group_device ;
	
	
    
    if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        printk(KERN_ERR  "Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -1;
    }

    devno = MKDEV(dev_major, groupno);
	snprintf(devname, "aosv_ipc_root%d", 16, groupno);


	/* Allocate space for cdev */
	group_cdevs[groupno-1] = cdev_alloc();
	if (group_cdevs[groupno-1] < 0) {
		printk(KERN_ERR  "Failed allocating space for cdev struct\n");
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}


	/*  Registering device to the kernel */
	cdev_init(group_cdevs[groupno-1], &ipc_group_ops);
	group_cdevs[groupno-1] -> owner = THIS_MODULE;
	res = cdev_add(group_cdevs[groupno-1], devno, 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}

	
	

	/*  Creating the device into the pseudo file system */
	group_device = device_create(dev_class, NULL, devno, NULL, devname);
	if (group_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}

	return 0;
	

DEVICE_CREATE_FAIL:
	cdev_del(group_cdevs[groupno-1]);

CDEV_ADD_FAIL:
	
	kfree(group_cdevs[groupno-1]);

CDEV_ALLOC_FAIL:


	return res;

}

int   ipc_group_uninstall(group_t groupno, int dev_major)
{
	dev_t devno;

    devno = MKDEV(dev_major, groupno);


	device_destroy(dev_class, devno);
	cdev_del(group_cdevs[groupno-1]);
	kfree(group_cdevs[groupno-1]);

	return 0;
}