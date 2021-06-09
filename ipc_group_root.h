#ifndef IPC_GROUP_ROOT_H
#define IPC_GROUP_ROOT_H

#include <linux/cdev.h>

#include "ipc_module_costants.h"

typedef struct ipc_group_root_dev_t {
	struct mutex lock;
	struct cdev cdev;
} ipc_group_root_dev;

int ipc_group_root_install(void);
int ipc_group_root_uninstall(void);

int ipc_group_install(group_t groupno);
int ipc_group_uninstall(group_t groupno);



#endif
