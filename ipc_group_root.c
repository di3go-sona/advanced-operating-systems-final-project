#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>


#include "ipc_module_costants.h"
#include "ipc_group_root.h"
#include "ipc_group.h"


struct device*	group_root_device ;


extern struct class*  	group_dev_class ;
extern struct cdev* 	group_root_cdev ;
extern struct cdev* 	group_cdevs[IPC_MAX_GROUPS+1];
extern dev_t            group_root_devno;

DEFINE_MUTEX(group_root_lock);



int ipc_group_root_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "ipc dev open");

	return 0;
}

long int ipc_group_root_ioctl(struct file *file, 
						 unsigned int ioctl_num,    
						 unsigned long ioctl_param){
	int res;
	mutex_lock(&group_root_lock);

    printk(KERN_INFO "ipc dev ioctl\n");

	if (ioctl_num == IPC_GROUP_INSTALL){
		res = ipc_group_install((group_t) ioctl_param);
	} else if  (ioctl_num == IPC_GROUP_UNINSTALL){
		res = ipc_group_uninstall((group_t) ioctl_param);
	} else {
		printk(KERN_INFO "unrecognized");
		res = -1;
	}

	mutex_unlock(&group_root_lock);
	return res;
}

int ipc_group_root_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "ipc dev release");

	return 0;
}

struct file_operations ipc_group_root_ops = {
	.open = ipc_group_root_open,
	.unlocked_ioctl = ipc_group_root_ioctl,
	.release = ipc_group_root_release
};



int ipc_group_root_install(void)
{
	int res;
    printk(KERN_INFO "ipc dev installing");

	/* Register device major and minor */
	res = alloc_chrdev_region(&group_root_devno, 0, IPC_MAX_GROUPS+1, IPC_ROOT_DEV_NAME);
	if (res < 0) {
		printk(KERN_ERR  "Failed registering char device\n");
		goto ALLOC_CHRDEV_REGION_FAIL;
	} else {
		printk(KERN_INFO "ipc dev installed with major %d and minor %d", MAJOR(group_root_devno), MINOR(group_root_devno));
	}

	/* Create a class : appears at /sys/class */
    group_dev_class  = class_create(THIS_MODULE, IPC_CLASS_NAME);
	if (group_dev_class == NULL) {
		printk(KERN_ERR  "Failed creating class\n");
		goto CLASS_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Class creation success");
	}

	/* Allocate space for cdev */
	group_root_cdev = cdev_alloc();
	if (group_root_cdev < 0) {
		printk(KERN_ERR  "Failed allocating space for cdev struct\n");
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}


	/*  Registering device to the kernel */
	cdev_init(group_root_cdev, &ipc_group_root_ops);
	group_root_cdev -> owner = THIS_MODULE;
	res = cdev_add(group_root_cdev, group_root_devno, 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}


	/*  Creating the device into the pseudo file system */
	group_root_device = device_create(group_dev_class, NULL, group_root_devno, NULL, IPC_ROOT_DEV_NAME);
	if (group_root_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}

	return 0;
	
	

// device_destroy(group_dev_class, group_root_device);

DEVICE_CREATE_FAIL:
	cdev_del(group_root_cdev);

CDEV_ADD_FAIL:
	
	kfree(group_root_cdev);

CDEV_ALLOC_FAIL:
	class_destroy(group_dev_class);
	
CLASS_CREATE_FAIL:
	unregister_chrdev_region(group_root_devno, IPC_MAX_GROUPS+1);

ALLOC_CHRDEV_REGION_FAIL:
	return res;

}

int ipc_group_root_uninstall(void)
{
	device_destroy(group_dev_class, group_root_devno);
	cdev_del(group_root_cdev);
	kfree(group_root_cdev);
	class_destroy(group_dev_class);
	unregister_chrdev_region(group_root_devno, IPC_MAX_GROUPS+1);
	return 0;
}


