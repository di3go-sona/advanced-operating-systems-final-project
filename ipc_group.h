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
	struct mutex lock;
	struct mutex delayed_lock;
	struct cdev cdev;
} ipc_group_dev;




typedef struct ipc_message_t {
	struct list_head next;
	char* payload;
	ssize_t payload_len;
	ipc_group_dev* group_dev;
	struct hrtimer timer;
} ipc_message;



int revoke_delayed_messages(ipc_group_dev* group_dev);





#endif