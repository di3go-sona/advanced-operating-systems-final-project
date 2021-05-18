#ifndef IPC_GROUP_H
#define IPC_GROUP_H

extern struct file_operations ipc_group_ops;

typedef struct ipc_message_t {
	struct list_head next;
	unsigned int size;
	char* payload;
} ipc_message;

typedef struct ipc_group_dev_t {
	struct list_head list;
	int subscribers;
	int msg_count;
	struct cdev cdev;
} ipc_group_dev;


#endif