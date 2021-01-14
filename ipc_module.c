#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diego Sonaglia");
MODULE_DESCRIPTION("An ipc message passing/synching module for the AOSV course @ La sapieza");
MODULE_VERSION("0.01");


#define IPC_DEV_MAJOR 117
#define IPC_DEV_NAME "group"


struct class* group_class ;
struct device* group_dev0;
int dev_major;

int ipc_group_open(struct inode *inode, struct file *filp) {

    printk("ipc dev open");

	return 0;
}
ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {

    printk("ipc dev read");

	return 0;
}

int ipc_group_release(struct inode *inode, struct file *filp)
{
    printk("ipc dev release");

	return 0;
}

struct file_operations ipc_group_ops = {
	.open = ipc_group_open,
	.read = ipc_group_read,
	.release = ipc_group_release
};


static int ipc_group_install(void)
{
	int err = 0;
    printk("ipc dev installing");
	
	/* Register device major and minor */
	dev_major = register_chrdev(0, IPC_DEV_NAME, &ipc_group_ops);
	if (dev_major < 0) {
		printk(KERN_ERR  "Failed registering char device\n");
	} else {
		printk(KERN_INFO "ipc dev installed with major :%d", dev_major);

	}

	/* Create a class : appears at /sys/class */
    group_class  = class_create(THIS_MODULE, "ipc_group");
	if (group_class == NULL) {
		printk(KERN_ERR  "Failed creating class\n");
	} else {
		printk(KERN_INFO "Class creation success");
	}

	struct device* group_dev0 = device_create(group_class, NULL, MKDEV(dev_major, 0), NULL, IPC_DEV_NAME);
	if (group_dev0 < 0) {
		printk(KERN_ERR  "Failed creating device\n");
	} else {
		printk(KERN_INFO "Device creation success");
	}


	return err;
}

static int   ipc_group_uninstall(void)
{
	int err;
    printk("ipc dev uninstalling");

	device_destroy(group_class,MKDEV(dev_major, 0));
	class_destroy(group_class);
	unregister_chrdev(IPC_DEV_MAJOR, IPC_DEV_NAME);


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
