#ifndef IPC_GROUP_H
#define IPC_GROUP_H

extern struct file_operations ipc_group_ops;

typedef struct ipc_message_t {
	struct list_head next;
	char* payload;
	ssize_t payload_len;
} ipc_message;





typedef struct ipc_group_dev_t {
	struct list_head msg_list;
	int subscribers;
	int msg_count;
	int threads_count;
	int closed;
	struct mutex lock;
	struct cdev cdev;
} ipc_group_dev;


#endif