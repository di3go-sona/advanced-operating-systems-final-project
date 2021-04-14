#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>

#include "ipc_module_costants.h"


extern struct class*  	group_dev_class ;
extern struct cdev* 	group_cdevs[IPC_MAX_GROUPS+1];
extern dev_t            group_root_devno;


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




int ipc_group_install(group_t groupno)
{
	int res;

	int dev_major = MAJOR(group_root_devno);
	dev_t group_devno;
	char devname[32] = {0};
	struct device*	group_device ;
	
	
    
    if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        printk(KERN_ERR  "Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -INVALID_GROUP_NUM;
    } else if(group_cdevs[groupno] != NULL) {
		return -GROUP_ALREADY_INSTALLED;
	}




    group_devno = MKDEV(dev_major, groupno);
	snprintf(devname,32, "aosv_ipc_group%d\n" , groupno);


	/* Allocate space for cdev */
	group_cdevs[groupno] = cdev_alloc();
	if (group_cdevs[groupno] < 0) {
		printk(KERN_ERR  "Failed allocating space for cdev struct\n");
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}


	/*  Registering device to the kernel */
	cdev_init(group_cdevs[groupno], &ipc_group_ops);
	group_cdevs[groupno] -> owner = THIS_MODULE;
	res = cdev_add(group_cdevs[groupno], group_devno, 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}

	
	

	/*  Creating the device into the pseudo file system */
	group_device = device_create(group_dev_class, NULL, group_devno, NULL, devname);
	if (group_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}

	return 0;
	

DEVICE_CREATE_FAIL:
	cdev_del(group_cdevs[groupno]);

CDEV_ADD_FAIL:
	
	kfree(group_cdevs[groupno]);

CDEV_ALLOC_FAIL:


	return res;

}

int   ipc_group_uninstall(group_t groupno)
{
	dev_t group_devno;
	int dev_major = MAJOR(group_root_devno);

    group_devno = MKDEV(dev_major, groupno);

	if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        printk(KERN_ERR  "Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -INVALID_GROUP_NUM;
    } else if(group_cdevs[groupno] == NULL) {
		return -GROUP_NOT_INSTALLED;
	}


	device_destroy(group_dev_class, group_devno);
	cdev_del(group_cdevs[groupno]);
	kfree(group_cdevs[groupno]);

	group_cdevs[groupno] = NULL;

	return 0;
}