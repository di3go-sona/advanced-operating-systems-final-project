#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h> 

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diego Sonaglia");
MODULE_DESCRIPTION("An ipc message passing/synching module for the AOSV course @ La sapieza");
MODULE_VERSION("0.01");


#define IPC_MAX_GROUPS 16
#define IPC_ROOT_DEV_NAME "aosv_ipc_root"
#define IPC_DEV_NAME "aosv_ipc_group"
#define IPC_CLASS_NAME "aosv_ipc_class"

struct class*  	dev_class ;
struct device*	root_device ;
struct cdev* 	root_cdev ;

dev_t devno;

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


static int ipc_group_install(void)
{
	int res;
    printk(KERN_INFO "ipc dev installing");

	/* Register device major and minor */
	res = alloc_chrdev_region(&devno, 0, IPC_MAX_GROUPS, IPC_ROOT_DEV_NAME);
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
	root_cdev = cdev_alloc();
	if (root_cdev < 0) {
		printk(KERN_ERR  "Failed allocating space for cdev struct\n");
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}


	/*  Registering device to the kernel */
	cdev_init(root_cdev, &ipc_group_ops);
	root_cdev -> owner = THIS_MODULE;
	res = cdev_add(root_cdev, devno, 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}


	/*  Creating the device into the pseudo file system */
	root_device = device_create(dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME);
	if (root_device < 0) {
		printk(KERN_ERR  "Failed creating device\n");
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device creation success");
	}

	return 0;
	
	

// device_destroy(dev_class, root_device);

DEVICE_CREATE_FAIL:
	cdev_del(root_cdev);

CDEV_ADD_FAIL:
	
	kfree(root_cdev);

CDEV_ALLOC_FAIL:
	class_destroy(dev_class);
	
CLASS_CREATE_FAIL:
	unregister_chrdev_region(devno, IPC_MAX_GROUPS);

ALLOC_CHRDEV_REGION_FAIL:
	return res;

}

static int   ipc_group_uninstall(void)
{
	device_destroy(dev_class, devno);
	cdev_del(root_cdev);
	kfree(root_cdev);
	class_destroy(dev_class);
	unregister_chrdev_region(devno, IPC_MAX_GROUPS);
	return 0;
}


static int ipc_init(void) {
    printk(KERN_INFO "ipc init");
    ipc_group_install();
    return 0;
}

static void  ipc_exit(void) {
	printk(KERN_INFO "ipc exit");
	ipc_group_uninstall();
}


module_init(ipc_init);
module_exit(ipc_exit);
