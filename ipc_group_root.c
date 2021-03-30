#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>


#include "ipc_module_costants.h"
#include "ipc_group_root.h"
#include "ipc_group.h"

struct class*  	dev_class ;
struct device*	root_group_device ;
struct cdev* 	root_group_cdev ;

dev_t devno;


int ipc_root_group_open(struct inode *inode, struct file *filp) {
	
    printk(KERN_INFO "ipc dev open");

	return 0;
}

long int ipc_root_group_ioctl(struct file *file, 
						 unsigned int ioctl_num,    
						 unsigned long ioctl_param){

    printk(KERN_INFO "ipc dev ioctl\n");
	if (ioctl_num == IPC_GROUP_INSTALL){
		return ipc_group_install((group_t) ioctl_param, ioctl_param);
	} else if  (ioctl_num == IPC_GROUP_UNINSTALL){
		return ipc_group_uninstall((group_t) ioctl_param, ioctl_param);
	} else {
		printk(KERN_INFO "unrecognized");
		return -1;
	}
}

int ipc_root_group_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "ipc dev release");

	return 0;
}

struct file_operations ipc_root_group_ops = {
	.open = ipc_root_group_open,
	.unlocked_ioctl = ipc_root_group_ioctl,
	.release = ipc_root_group_release
};



int ipc_group_root_install(void)
{
	int res;
    printk(KERN_INFO "ipc dev installing");

	/* Register device major and minor */
	res = alloc_chrdev_region(&devno, 0, IPC_MAX_GROUPS+1, IPC_ROOT_DEV_NAME);
	if (res < 0) {
		printk(KERN_ERR  "Failed registering char device\n");
		goto ALLOC_CHRDEV_REGION_FAIL;
	} else {
		printk(KERN_INFO "ipc dev installed with major %d and minor %d", MAJOR(devno), MINOR(devno));
	}

	/* Create a class : appears at /sys/class */
    dev_class  = class_create(THIS_MODULE, IPC_CLASS_NAME);
	if (dev_class == NULL) {
		printk(KERN_ERR  "Failed creating class\n");
		goto CLASS_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Class creation success");
	}

	/* Allocate space for cdev */
	root_group_cdev = cdev_alloc();
	if (root_group_cdev < 0) {
		printk(KERN_ERR  "Failed allocating space for cdev struct\n");
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}


	/*  Registering device to the kernel */
	cdev_init(root_group_cdev, &ipc_root_group_ops);
	root_group_cdev -> owner = THIS_MODULE;
	res = cdev_add(root_group_cdev, devno, 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}


	/*  Creating the device into the pseudo file system */
	root_group_device = device_create(dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME);
	if (root_group_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}

	return 0;
	
	

// device_destroy(dev_class, root_group_device);

DEVICE_CREATE_FAIL:
	cdev_del(root_group_cdev);

CDEV_ADD_FAIL:
	
	kfree(root_group_cdev);

CDEV_ALLOC_FAIL:
	class_destroy(dev_class);
	
CLASS_CREATE_FAIL:
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);

ALLOC_CHRDEV_REGION_FAIL:
	return res;

}

int ipc_group_root_uninstall(void)
{
	device_destroy(dev_class, devno);
	cdev_del(root_group_cdev);
	kfree(root_group_cdev);
	class_destroy(dev_class);
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);
	return 0;
}


