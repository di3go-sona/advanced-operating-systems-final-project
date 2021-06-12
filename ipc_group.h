#ifndef IPC_GROUP_H
#define IPC_GROUP_H

extern struct file_operations ipc_group_ops;


typedef struct ipc_group_dev_t {
	struct list_head msg_list;
	struct list_head delayed_msg_list;
	int msg_count;
	int delayed_msg_count;
	int threads_count;
	ktime_t delay;
	spinlock_t lock;
	spinlock_t delayed_lock;
	struct wait_queue_head wait_queue;
	int waiting_count;
	int awaking_count;
	bool closing;
	struct cdev cdev;
} ipc_group_dev;

typedef struct ipc_group_root_dev_t {
	spinlock_t lock;
	bool closing;
	struct cdev cdev;
} ipc_group_root_dev;



typedef struct ipc_message_t {
	struct list_head next;
	char* payload;
	ssize_t payload_len;
	ipc_group_dev* group_dev;
	struct hrtimer timer;
} ipc_message;


extern int max_message_size;
extern int max_storage_size;

extern struct class*  	group_dev_class ;
extern int group_major;


extern ipc_group_root_dev* 	group_root_dev ;
extern ipc_group_dev* 		group_devs[IPC_MAX_GROUPS+1];

int ipc_group_install(group_t groupno);
int ipc_group_uninstall(group_t groupno);




#endif