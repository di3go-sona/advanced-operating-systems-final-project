#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>

#include <linux/list.h>
#include "ipc_module_costants.h"
#include "ipc_group_root.h"
#include "ipc_group.h"


#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#define DEBUG(...) do{  printk( KERN_INFO "[ DEBUG ] " __VA_ARGS__ );} while( 0 )
#else
#define DEBUG(...) do{ } while ( 0 )
#endif


struct class*  	group_dev_class ;
int group_major;
int group_minor;

struct cdev* 	group_root_cdev ;
ipc_group_dev* 	group_devs[IPC_MAX_GROUPS+1] = {0};

DEFINE_MUTEX(group_root_lock);


int ipc_group_root_open(struct inode *inode, struct file *filp) {
    printk(KERN_INFO "root ipc dev open");

	return 0;
}

long int ipc_group_root_ioctl(struct file *file, 
						 unsigned int ioctl_num,    
						 unsigned long ioctl_param){
	int res;
	mutex_lock(&group_root_lock);

    printk(KERN_INFO "root ipc dev ioctl\n");

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
    printk(KERN_INFO "root ipc dev release");

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
	dev_t devno;
	struct device* group_root_device;

	printk(KERN_INFO "ipc dev installing");


	/* Register device major and minor */
	res = alloc_chrdev_region(&devno, 0, IPC_MAX_GROUPS+1, IPC_ROOT_DEV_NAME);
	if (res < 0) {
		printk(KERN_ERR  "Failed registering char device\n");
		goto ALLOC_CHRDEV_REGION_FAIL;
	} else {
		printk(KERN_INFO "ipc dev installed with major %d and minor %d", MAJOR(devno), MINOR(devno));
		group_major = MAJOR(devno);
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
	res = cdev_add(group_root_cdev,devno , 1);
	if (res < 0) {
		printk(KERN_ERR  "Failed making cdev live \n");
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "cdev is now live");
	}


	/*  Creating the device into the pseudo file system */
	group_root_device = device_create(group_dev_class, NULL, devno, NULL, IPC_ROOT_DEV_NAME);
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
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);

ALLOC_CHRDEV_REGION_FAIL:
	return res;

}

int ipc_group_root_uninstall(void)
{
	dev_t devno = MKDEV(group_major, 0);

	device_destroy(group_dev_class, devno);
	cdev_del(group_root_cdev);
	kfree(group_root_cdev);
	class_destroy(group_dev_class);
	unregister_chrdev_region(devno, IPC_MAX_GROUPS+1);
	return 0;
}




int ipc_group_install(group_t groupno)
{
	int res;

	dev_t group_devno;
	char devname[IPC_DEV_NAMESIZE] = {0};
	struct device*	group_device ;
	ipc_group_dev* group_dev;
	printk(KERN_INFO  "Installing group %d", groupno);

	
    
    if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        printk(KERN_ERR  "Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -INVALID_GROUP_NUM;
    } else if(group_devs[groupno] != NULL) {
		return -GROUP_ALREADY_INSTALLED;
	}




    group_devno = MKDEV(group_major, groupno);
	snprintf(devname,IPC_DEV_NAMESIZE, "aosv_ipc_dev%d" , groupno);
	group_dev = kmalloc(sizeof(ipc_group_dev), 0);



	/* Allocate space for cdev */
	if (group_dev == NULL) {
		printk(KERN_ERR "Device %d allocation failed", groupno);
		goto CDEV_ALLOC_FAIL;
	} else {
		printk(KERN_INFO "Device %d allocation ok", groupno);
	}


	/*  Registering device to the kernel */
	cdev_init(&(group_dev->cdev), &ipc_group_ops);
	mutex_init(&(group_dev->lock));
	mutex_init(&(group_dev->delayed_lock));
	group_dev -> cdev.owner = THIS_MODULE;
	group_dev -> msg_count = 0;
	group_dev -> delay = 10;

	INIT_LIST_HEAD(&( group_dev -> msg_list ));
	INIT_LIST_HEAD(&( group_dev -> delayed_msg_list ));
	

	res = cdev_add(&(group_dev->cdev), group_devno, 1);
	if (res < 0) {
		printk(KERN_ERR "Device %d add failed", groupno);
		goto CDEV_ADD_FAIL;
	} else {
		printk(KERN_INFO "Device %d add successful", groupno);
	}

	
	

	/*  Creating the device into the pseudo file system */
	group_device = device_create(group_dev_class, NULL, group_devno, NULL, devname);
	if (group_device < 0) {
		printk(KERN_ERR "Device %d creation failed", groupno);
		goto DEVICE_CREATE_FAIL;
	} else {
		printk(KERN_INFO "Device %d creation success", groupno);
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

int ipc_group_uninstall(group_t groupno)
{
	dev_t group_devno;
	ipc_group_dev* group_dev;

	DEBUG( "Uninstalling group %d", groupno);

	group_devno = MKDEV(group_major, groupno);
	
	if (groupno < 1 || groupno > IPC_MAX_GROUPS){
        printk(KERN_ERR  "Invalid group number, min is 1 and max is %d", IPC_MAX_GROUPS);
        return -INVALID_GROUP_NUM;
    } else if(group_devs[groupno] == NULL) {
		return -GROUP_NOT_INSTALLED;
	} else {
		group_dev = group_devs[groupno];
	}

	if (group_dev -> threads_count != 0 ) return -GROUP_NOT_EMPTY;
	if (group_dev -> delayed_msg_count != 0 ) return -GROUP_NOT_EMPTY;


	device_destroy(group_dev_class, group_devno);
	cdev_del(&(group_dev->cdev));
	kfree(group_dev);

	group_devs[groupno] = NULL;

	return 0;
}