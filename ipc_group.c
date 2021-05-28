#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>
#include <linux/list.h>


#include "ipc_module_costants.h"
#include "ipc_group.h"

#define DEBUG_ENABLED
#ifdef DEBUG_ENABLED
#define DEBUG(...) do{  printk( KERN_INFO "[ DEBUG ] " __VA_ARGS__ );} while( 0 )
#else
#define DEBUG(...) do{ } while ( 0 )
#endif


int ipc_group_open(struct inode *inode, struct file *filp) {
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);
	group_dev -> threads_count ++ ;


	return 0;
}
// ssize_t ipc_group_ioctl (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {
// 	struct cdev *group_cdev;
// 	ipc_group_dev *group_dev;
// 	ipc_message *msg;
// 	ssize_t copied = 0, to_copy; 
// 	long res = 0;

// 	DEBUG("reading msg ");


// 	group_cdev = filp->f_inode->i_cdev;
// 	group_dev = container_of(group_cdev, ipc_group_dev, cdev);
// }

ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	ssize_t copied = 0, to_copy; 
	long res = 0;

	DEBUG("reading msg ");


	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	mutex_lock( &(group_dev ->lock));
	
	if (group_dev -> msg_count == 0){
		mutex_unlock( &(group_dev ->lock));
		DEBUG("no msg found");

		return -NO_MESSAGES;
	}

	
	msg = list_first_entry(&(group_dev -> msg_list), ipc_message, next);
	(group_dev -> msg_count )-- ;

	__list_del_entry(&(msg->next));
	mutex_unlock( &(group_dev ->lock));

	to_copy = lrn > msg -> payload_len ? msg -> payload_len : lrn;

	while( copied < to_copy){
		res = copy_to_user(buf, msg->payload, to_copy-copied);
		if (res >0) copied += res;
		else if (copied ==0) break;
	}

	// DEBUG("freeing %p", msg->payload);
	kfree(msg->payload);
	// DEBUG("freeing %p", msg);
	kfree(msg);

	DEBUG("read msg \"%.*s\" ", (int)lrn, buf);

	return SUCCESS;


}

static int _publish_delayed_message(ipc_message* msg, ipc_group_dev* group_dev){
	DEBUG("publishing delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);

	mutex_lock( &(group_dev ->delayed_lock));
	(group_dev -> delayed_msg_count )-- ;
	__list_del_entry(&(msg->next));
	mutex_unlock( &(group_dev ->delayed_lock));
	DEBUG("dequeued delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);


	mutex_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	mutex_unlock( &(group_dev ->lock));
	DEBUG("published delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);

	return SUCCESS;
}

static int _revoke_delayed_message(ipc_message* msg, ipc_group_dev* group_dev){
	struct hrtimer *timer = &(msg->timer);

	DEBUG("revoking delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);
	hrtimer_cancel(timer);
	// if ( !del_timer( timer ) ) return -TIMER_DEL_FAILED;

	mutex_lock( &(group_dev ->lock));
	(group_dev -> delayed_msg_count )-- ;
	__list_del_entry(&(msg->next));
	mutex_unlock( &(group_dev ->lock));

	return SUCCESS;
}

int revoke_delayed_messages(ipc_group_dev* group_dev){


	return SUCCESS;
}

static enum hrtimer_restart _publish_delayed_message_handler(struct hrtimer *timer)
{
    ipc_message* msg;
	msg = container_of(timer, ipc_message, timer);
	_publish_delayed_message(msg, msg ->group_dev);
    return HRTIMER_NORESTART;  //restart timer
}

static int _enqueue_message(ipc_message* msg, ipc_group_dev* group_dev){
	DEBUG("enqueuing msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);

	mutex_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	mutex_unlock( &(group_dev ->lock));
	return SUCCESS;
}

static int _enqueue_delayed_message(ipc_message* msg, ipc_group_dev* group_dev){
	struct hrtimer *timer = &(msg->timer);

	DEBUG("enqueuing delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);


	mutex_lock( &(group_dev ->delayed_lock));
	hrtimer_init(timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer -> function = _publish_delayed_message_handler;
	hrtimer_start(timer,group_dev -> delay,HRTIMER_MODE_REL);

	(group_dev -> delayed_msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> delayed_msg_list));
	mutex_unlock( &(group_dev ->delayed_lock));


	




	return SUCCESS;
}

ssize_t ipc_group_write (struct file * filp, const char __user * buf , size_t lrn , loff_t *offset){
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	char *payload_buf; 
	ssize_t copied =0 ;
	long res = 0;
		
	DEBUG("writing msg \"%.*s\" ", (int)lrn, buf);


	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);


	// TODO: check lrn < MAX_IPC_MSG_SIZE
	msg = kmalloc(sizeof(ipc_message), GFP_USER);
	if (msg == NULL) {
		res = -MEM_ALLOCATION_FAILED;
		goto MSG_ALLOC_FAIL;
	};

	payload_buf = kmalloc(lrn, GFP_USER);
	if (payload_buf == NULL) {
		res = -MEM_ALLOCATION_FAILED;
		goto MSG_PAYLOAD_ALLOC_FAIL;
	};

	msg -> payload_len = lrn;
	msg -> payload = payload_buf;
	msg -> group_dev = group_dev;
	copied = 0; 

	while( copied < lrn){
		res = copy_from_user(payload_buf, buf, lrn-copied);
		if (res >0) copied += res;
		else if (copied ==0) break;
	}

	// mutex_lock( &(group_dev ->lock));
	// (group_dev -> msg_count )++ ;
	// list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	// mutex_unlock( &(group_dev ->lock));
	if (group_dev -> delay == 0){
		res = _enqueue_message(msg, group_dev);
	} else {
		res = _enqueue_delayed_message(msg, group_dev);
	}
	if (res != SUCCESS) {
		goto MSG_ENQUEUE_FAIL;
	};
	return SUCCESS;

MSG_ENQUEUE_FAIL:
	kfree(msg-> payload);
	
MSG_PAYLOAD_ALLOC_FAIL:
	kfree(msg);

MSG_ALLOC_FAIL:
	return res;


}

int ipc_group_release(struct inode *inode, struct file *filp)
{
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;




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


