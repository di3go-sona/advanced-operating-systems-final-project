#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>

#include <linux/list.h>
#include "ipc_module_costants.h"
#include "ipc_kernel_macros.h"
#include "ipc_group_root.h"
#include "ipc_group.h"




int ipc_group_root_open(struct inode *inode, struct file *filp) {
	return SUCCESS;
}

long int ipc_group_root_ioctl(struct file *filp, 
						 unsigned int ioctl_num,    
						 unsigned long ioctl_param){
	int res;
	GR_DEBUG( "ioctl %d %ld", ioctl_num, ioctl_param);

	spin_lock(&(group_root_dev -> lock));

	switch (ioctl_num)
	{
	case IPC_GROUP_INSTALL:
		res = ipc_group_install((group_t) ioctl_param);
		break;

	case IPC_GROUP_UNINSTALL:
		res = ipc_group_uninstall((group_t) ioctl_param);
		break;
	
	default:
		GR_DEBUG( "unrecognized %d", ioctl_num);
		break;
	}

	spin_unlock(&(group_root_dev -> lock));
	return res;
}

int ipc_group_root_release(struct inode *inode, struct file *filp)
{
    GR_DEBUG( "root ipc dev release");
	return SUCCESS;
}


struct file_operations ipc_group_root_ops = {
	.open = ipc_group_root_open,
	.unlocked_ioctl = ipc_group_root_ioctl,
	.release = ipc_group_root_release
};

int ipc_group_root_install(void)
{
	int res;
	dev_t devno;
	struct device* group_root_device;
	GR_DEBUG( "installing");


	/* Register device major and minor */
	res = alloc_chrdev_region(&devno, 0, IPC_MAX_GROUPS+1, IPC_ROOT_DEV_NAME);
	if (res < 0) {
		printk(KERN_ERR  "alloc_chrdev_region failed\n");
		goto ALLOC_CHRDEV_REGION_FAIL;
	} else {
		GR_DEBUG( "alloc_chrdev_region: major %d and minor %d", MAJOR(devno), MINOR(devno));
		group_major = MAJOR(devno);
	}

	/* Create a class : appears at /sys/class */
    group_dev_class  = class_create(THIS_MODULE, IPC_CLASS_NAME);
	if (group_dev_class == NULL) {
		printk(KERN_ERR  "Failed creating class\n");
		goto CLASS_CREATE_FAIL;
	} else {
		GR_DEBUG( "Class creation success");
	}

	/*  Allocating memory */
	group_root_dev = kmalloc(sizeof(ipc_group_root_dev), GFP_USER);
	if (group_root_dev == NULL) {
		printk(KERN_ERR  "kmalloc ko: %p", group_root_dev);
		goto CDEV_ALLOC_FAIL;
	} else {
		GR_DEBUG( "kmalloc ok: %p", group_root_dev);
	}
	
	/*  Initializing device values */
	cdev_init(&(group_root_dev -> cdev) , &ipc_group_root_ops);
	spin_lock_init(&(group_root_dev->lock));
	group_root_dev -> cdev.owner = THIS_MODULE;

	/*  Registering device to the kernel */
	res = cdev_add(&(group_root_dev -> cdev) ,devno , 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		GR_DEBUG( "cdev is now live");
	}


	/*  Creating the device into the pseudo file system */
	group_root_device = device_create(group_dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME);
	if (group_root_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		GR_DEBUG( "Device creation success");
	}

	return 0;
	
	

// device_destroy(group_dev_class, group_root_device);

DEVICE_CREATE_FAIL:
	cdev_del(&(group_root_dev -> cdev));

CDEV_ADD_FAIL:
	kfree(group_root_dev);

CDEV_ALLOC_FAIL:
	class_destroy(group_dev_class);
	
CLASS_CREATE_FAIL:
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);

ALLOC_CHRDEV_REGION_FAIL:
	return res;

}

int ipc_group_root_uninstall(void)
{
	dev_t devno;
	GR_DEBUG( "uninstalling");
	devno = MKDEV(group_major, 0);
	GR_DEBUG( "device_destroy");
	device_destroy(group_dev_class, devno);
	GR_DEBUG( "cdev_del");
	cdev_del(&(group_root_dev -> cdev));
	GR_DEBUG( "kfree %p",group_root_dev );
	kfree(group_root_dev);
	GR_DEBUG( "class_destroy" );
	class_destroy(group_dev_class);
	GR_DEBUG( "unregister_chrdev_region");
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);
	GR_DEBUG( "uninstalled");
	return SUCCESS;
}


int ipc_group_install(group_t groupno)
{
	int res;

	dev_t group_devno;
	char devname[IPC_DEV_NAMESIZE] = {0};
	struct device*	group_device ;
	ipc_group_dev* group_dev;
	GR_DEBUG(  "Installing group %d", groupno);

	
    
    if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        GR_DEBUG("Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -INVALID_GROUP_NUM;
    } else if(group_devs[groupno] != NULL) {
		GR_DEBUG("Group already installed");
		return -GROUP_ALREADY_INSTALLED;
	}




    group_devno = MKDEV(group_major, groupno);
	snprintf(devname,IPC_DEV_NAMESIZE, "aosv_ipc_dev%d" , groupno);
	group_dev = kmalloc(sizeof(ipc_group_dev), GFP_USER);



	/* Allocate space for cdev */
	if (group_dev == NULL) {
		printk(KERN_ERR "Device %d allocation failed", groupno);
		goto CDEV_ALLOC_FAIL;
	} else {
		GR_DEBUG( "Device %d allocation ok", groupno);
	}


	/*  Registering device to the kernel */
	cdev_init(&(group_dev->cdev), &ipc_group_ops);
	spin_lock_init(&(group_dev->lock));
	spin_lock_init(&(group_dev->delayed_lock));
	group_dev -> cdev.owner = THIS_MODULE;
	group_dev -> msg_count = 0;
	group_dev -> delay = ktime_set(0,0);
	group_dev -> waiting_count = 0;
	group_dev -> awaking_count = 0;
	init_waitqueue_head( &(group_dev -> wait_queue));

	INIT_LIST_HEAD(&( group_dev -> msg_list ));
	INIT_LIST_HEAD(&( group_dev -> delayed_msg_list ));
	

	res = cdev_add(&(group_dev->cdev), group_devno, 1);
	if (res < 0) {
		printk(KERN_ERR "Device %d add failed", groupno);
		goto CDEV_ADD_FAIL;
	} else {
		GR_DEBUG( "Device %d add successful", groupno);
	}

	
	

	/*  Creating the device into the pseudo file system */
	group_device = device_create(group_dev_class, NULL, group_devno, NULL, devname);
	if (group_device < 0) {
		printk(KERN_ERR "Device %d creation failed", groupno);
		goto DEVICE_CREATE_FAIL;
	} else {
		GR_DEBUG( "Device %d creation success", groupno);
	}

	group_devs[groupno] = group_dev;
	return 0;
	

DEVICE_CREATE_FAIL:
	cdev_del(&(group_dev->cdev));

CDEV_ADD_FAIL:
	kfree(group_dev);

CDEV_ALLOC_FAIL:
	group_devs[groupno] = NULL;
	return res;

}
