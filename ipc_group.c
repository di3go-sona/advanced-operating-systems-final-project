#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h> 
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/moduleparam.h>


#include <linux/wait.h>
#include "ipc_module_costants.h"
#include "ipc_kernel_macros.h"
#include "ipc_group.h"



int max_message_size = 16;
int max_storage_size = 4096;
int curr_storage_size = 0;

struct class*  	group_dev_class ;
int group_major;


ipc_group_root_dev* 	group_root_dev ;
ipc_group_dev* 			group_devs[IPC_MAX_GROUPS+1] = {0};

module_param(max_message_size, int, 0660);
module_param(max_storage_size, int, 0660);


static int _publish_delayed_message(ipc_message* msg, ipc_group_dev* group_dev){
	DEBUG("publishing delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);

	spin_lock( &(group_dev ->delayed_lock));
	(group_dev -> delayed_msg_count )-- ;
	__list_del_entry(&(msg->next));
	spin_unlock( &(group_dev ->delayed_lock));
	// DEBUG("dequeued delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);


	spin_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	spin_unlock( &(group_dev ->lock));
	// DEBUG("published delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);

	return SUCCESS;
}

static int _revoke_delayed_messages(ipc_group_dev* group_dev){

	struct list_head tmp_list;
	ipc_message* tmp_msg, *_tmp_msg;
	int tmp_count;
	int payload_len;

	INIT_LIST_HEAD(&tmp_list);

	spin_lock( &(group_dev ->delayed_lock));

	tmp_count = group_dev -> delayed_msg_count;
	if (tmp_count == 0 ) {
		DEBUG("No message to revoke");
	} else {
		group_dev -> delayed_msg_count = 0 ;
		DEBUG("Revoking %d delayed messages ",tmp_count);
		list_splice_init(  &(group_dev -> delayed_msg_list), &tmp_list );
	};

	spin_unlock( &(group_dev ->delayed_lock));

	list_for_each_entry_safe(tmp_msg, _tmp_msg, &tmp_list, next){
		hrtimer_cancel(&(tmp_msg->timer));
	}

	list_for_each_entry_safe(tmp_msg, _tmp_msg, &tmp_list, next){
		__list_del_entry(&(tmp_msg -> next) );
		payload_len = tmp_msg -> payload_len;
		kfree(tmp_msg -> payload);
		kfree(tmp_msg);
		__sync_sub_and_fetch( &curr_storage_size, payload_len + sizeof(ipc_message));

	}

	return SUCCESS;
}

static int _flush_delayed_messages(ipc_group_dev* group_dev){

	struct list_head tmp_list;
	ipc_message* tmp_msg, *_tmp_msg;
	int tmp_count;

	INIT_LIST_HEAD(&tmp_list);

	spin_lock( &(group_dev ->delayed_lock));

	tmp_count = group_dev -> delayed_msg_count;
	if (tmp_count == 0 ) {
		DEBUG("No message to flush");
	} else {
		group_dev -> delayed_msg_count = 0 ;
		DEBUG("Flushing %d delayed messages ",tmp_count);
		list_splice_init(  &(group_dev -> delayed_msg_list), &tmp_list );
	};

	spin_unlock( &(group_dev ->delayed_lock));


	list_for_each_entry_safe(tmp_msg, _tmp_msg, &tmp_list, next){
		hrtimer_cancel(&(tmp_msg->timer));
	}

	spin_lock( &(group_dev ->lock));
	list_splice( &tmp_list,  &(group_dev -> msg_list) );
	group_dev -> msg_count += tmp_count ;
	spin_unlock( &(group_dev ->lock));

	return SUCCESS;
}

static int _sleep_on_barrier(ipc_group_dev* group_dev){
	int pos;


	
	pos = __sync_add_and_fetch( &(group_dev -> waiting_count), 1);
	DEBUG("barrier: sleeping pos %d", pos);

	wait_event_interruptible(group_dev -> wait_queue, group_dev -> awaking_count > 0);
	
	__sync_sub_and_fetch( &(group_dev -> awaking_count), 1);



	DEBUG("barrier: awoke pos %d", pos);


	return SUCCESS;
}

static int _delete_messages(ipc_group_dev* group_dev){

	ipc_message* tmp_msg, *_tmp_msg;

	list_for_each_entry_safe(tmp_msg, _tmp_msg, &(group_dev ->msg_list), next){
		kfree(tmp_msg -> payload);
		kfree(tmp_msg);
	}

	return SUCCESS;
}

static int _awake_barrier(ipc_group_dev* group_dev){
	int to_awake;
	DEBUG("barrier: wake up issued");

	// group_dev -> awaking = true;
	
	to_awake = __sync_fetch_and_and( &(group_dev -> waiting_count), 0);
	__sync_add_and_fetch( &(group_dev -> awaking_count), to_awake);
	wake_up_nr(&(group_dev -> wait_queue), to_awake);
	// group_dev -> awaking = false;
	

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

	spin_lock( &(group_dev ->lock));
	(group_dev -> msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	spin_unlock( &(group_dev ->lock));
	return SUCCESS;
}

static int _enqueue_delayed_message(ipc_message* msg, ipc_group_dev* group_dev){
	struct hrtimer *timer = &(msg->timer);

	DEBUG("enqueuing delayed msg \"%.*s\" ", (int)msg->payload_len, msg-> payload);


	spin_lock( &(group_dev ->delayed_lock));
	hrtimer_init(timer,CLOCK_MONOTONIC,HRTIMER_MODE_REL);
	timer -> function = _publish_delayed_message_handler;
	hrtimer_start(timer,group_dev -> delay,HRTIMER_MODE_REL);

	(group_dev -> delayed_msg_count )++ ;
	list_add_tail (	&(msg -> next), &(group_dev -> delayed_msg_list));
	spin_unlock( &(group_dev ->delayed_lock));

	return SUCCESS;
}


int ipc_group_open(struct inode *inode, struct file *filp) {
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;


	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);
	if (group_dev -> closing) return -GROUP_CLOSING;
	
	group_dev -> threads_count ++ ;
	return SUCCESS;
}

long int ipc_group_ioctl(struct file *filp, 
						 unsigned int ioctl_num,    
						 unsigned long ioctl_param){
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	long res = SUCCESS;

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	switch (ioctl_num)
	{
	case SET_SEND_DELAY:
		DEBUG("ioctl: setting delay of %d seconds",(int)ioctl_param );
		group_dev -> delay = ktime_set((int)ioctl_param,0);
		break;

	case REVOKE_DELAYED_MESSAGES:
		DEBUG("ioctl: revoke delayed messages" );
		_revoke_delayed_messages(group_dev);
		break;

	case FLUSH_DELAYED_MESSAGES:
		DEBUG("ioctl: flush delayed messages" );
		_flush_delayed_messages(group_dev);
		break;

	case SLEEP_ON_BARRIER:
		DEBUG("ioctl: sleep on barrier" );
		_sleep_on_barrier(group_dev);
		break;

	case AWAKE_BARRIER:
		DEBUG("ioctl: awake barrier" );
		_awake_barrier(group_dev);
		break;
		
	default:
		DEBUG( "ioctl: unrecognized %d", ioctl_num);
		res = -INVALID_IOCTL_NUM;
		break;
	}

	return res;	
}

ssize_t ipc_group_read (struct file * filp, char __user * buf , size_t lrn, loff_t *offset) {
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	ssize_t copied = 0, to_copy; 
	long res = 0;
	int payload_len;

	DEBUG("reading msg ");


	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	spin_lock( &(group_dev ->lock));
	
	if (group_dev -> msg_count == 0){
		spin_unlock( &(group_dev ->lock));
		DEBUG("no msg found");

		return -NO_MESSAGES;
	}

	
	msg = list_first_entry(&(group_dev -> msg_list), ipc_message, next);
	(group_dev -> msg_count )-- ;

	__list_del_entry(&(msg->next));
	spin_unlock( &(group_dev ->lock));

	to_copy = lrn > msg -> payload_len ? msg -> payload_len : lrn;

	while( copied < to_copy){
		res = copy_to_user(buf, msg->payload, to_copy-copied);
		if (res >0) copied += res;
		else if (copied ==0) break;
	}

	// DEBUG("freeing %p", msg->payload);
	payload_len = msg -> payload_len;
	kfree(msg->payload);
	// DEBUG("freeing %p", msg);
	kfree(msg);

	__sync_sub_and_fetch( &curr_storage_size, payload_len + sizeof(ipc_message));

	DEBUG("read msg \"%.*s\" ", (int)lrn, buf);

	return SUCCESS;


}

ssize_t ipc_group_write (struct file * filp, const char __user * buf , size_t lrn , loff_t *offset){
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;
	ipc_message *msg;
	char *payload_buf; 
	ssize_t copied =0 ;
	long res = 0;
	int old_storage_size, new_storage_size;
		
	DEBUG("writing msg \"%.*s\"", (int)lrn, buf);

	// Check if messafe size is less than maximum
	if (lrn > max_message_size) return -EFBIG;
	do{
		old_storage_size = curr_storage_size;
		new_storage_size = old_storage_size + lrn + sizeof(ipc_message);
		if (new_storage_size > max_storage_size) {
			DEBUG("No space left");
			return -ENOSPC;		
		}
		

	} while (__sync_bool_compare_and_swap (&curr_storage_size, old_storage_size, new_storage_size) == false );




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

	// spin_lock( &(group_dev ->lock));
	// (group_dev -> msg_count )++ ;
	// list_add_tail (	&(msg -> next), &(group_dev -> msg_list));
	// spin_unlock( &(group_dev ->lock));
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

int ipc_group_flush (struct file *filp,  fl_owner_t id){
	struct cdev *group_cdev;
	ipc_group_dev *group_dev;

	group_cdev = filp->f_inode->i_cdev;
	group_dev = container_of(group_cdev, ipc_group_dev, cdev);

	return _flush_delayed_messages(group_dev);
}

struct file_operations ipc_group_ops = {
	.open = ipc_group_open,
	.read = ipc_group_read,
	.write = ipc_group_write,
	.release = ipc_group_release,
	.unlocked_ioctl = ipc_group_ioctl,
	.flush = ipc_group_flush
};



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

	group_dev -> closing = true;
	while (group_dev -> threads_count > 0 ){};

	_revoke_delayed_messages( group_dev );
	_delete_messages( group_dev );
	device_destroy(group_dev_class, group_devno);
	cdev_del(&(group_dev->cdev));
	kfree(group_dev);

	group_devs[groupno] = NULL;

	return SUCCESS;
}