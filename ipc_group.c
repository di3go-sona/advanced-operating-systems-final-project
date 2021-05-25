#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>
#include <linux/list.h>

#include "ipc_module_costants.h"
#include "ipc_group.h"




int ipc_group_open(struct inode *inode, struct file *filp) {
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	
	printk(KERN_INFO "ipc dev open");

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);
	group_dev -> threads_count ++ ;


	return 0;
}


ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {

	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	ssize_t copied = 0, to_copy; 
	long res = 0;

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	mutex_lock( &(group_dev ->lock));
	(group_dev -> msg_count )-- ;
	msg = list_first_entry(&(group_dev -> msg_list), ipc_message, next);
	__list_del_entry(&(msg->next));
	mutex_unlock( &(group_dev ->lock));

	to_copy = lrn > msg -> payload_len ? msg -> payload_len : lrn;

	while( copied < to_copy){
		printk(KERN_INFO "ipc dev read, buf: %p, len: %lu, remaining: %lu", buf, to_copy, to_copy-copied);
		res = copy_to_user(buf, msg->payload, to_copy-copied);
		if (res >0) copied += res;
		else if (copied ==0) break;
	}


	kfree(msg->payload);
	kfree(msg);


	return SUCCESS;


}

ssize_t ipc_group_write (struct file * filp, const char __user * buf , size_t lrn , loff_t *offset){
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	char *payload_buf; 
	ssize_t copied =0 ;
	long res = 0;

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);


	// TODO: check lrn < MAX_IPC_MSG_SIZE
	msg = kmalloc(sizeof(ipc_message), GFP_USER);
	if (msg == NULL) goto MSG_ALLOC_FAIL;

	payload_buf = kmalloc(lrn, GFP_USER);
	if (payload_buf == NULL) goto MSG_PAYLOAD_ALLOC_FAIL;

	msg -> payload_len = lrn;
	msg -> payload = payload_buf;
	copied = 0; 

	while( copied < lrn){
		printk(KERN_INFO "ipc dev write, buf: %p, len: %lu, remaining: %lu", buf, lrn, lrn-copied);
		res = copy_from_user(payload_buf, buf, lrn-copied);
		if (res >0) copied += res;
		else if (copied ==0) break;
	}

	mutex_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	mutex_unlock( &(group_dev ->lock));


	return SUCCESS;

	
MSG_PAYLOAD_ALLOC_FAIL:
	kfree(msg);

MSG_ALLOC_FAIL:
	return -MEM_ALLOCATION_FAILED;


}





int ipc_group_release(struct inode *inode, struct file *filp)
{
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	printk(KERN_INFO "ipc dev release");



	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	(group_dev -> threads_count )-- ;
	

	return 0;
}

struct file_operations ipc_group_ops = {
	.open = ipc_group_open,
	.read = ipc_group_read,
	.write = ipc_group_write,
	.release = ipc_group_release
};


