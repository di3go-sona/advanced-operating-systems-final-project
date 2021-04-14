#ifndef IPC_COSTANTS_H
#define IPC_COSTANTS_H

#ifdef KERNELSPACE
// Kernel-space only
#include "linux/types.h"

extern struct class*  	group_dev_class ;
extern struct cdev* 	group_root_cdev ;
extern struct cdev* 	group_cdevs[IPC_MAX_GROUPS+1];
extern dev_t            group_root_devno;
#endif

#define IPC_MAX_GROUPS 16
#define IPC_ROOT_DEV_NAME "aosv_ipc_root"
#define IPC_DEV_NAME "aosv_ipc_group"
#define IPC_CLASS_NAME "aosv_ipc_class"

#define IPC_GROUP_INSTALL 0
#define IPC_GROUP_UNINSTALL 1

typedef unsigned int group_t;
typedef enum {
    SUCCESS = 0,
    CANNOT_OPEN_GROUP_ROOT = 1,
    INVALID_GROUP_NUM = 2,
    GROUP_NOT_INSTALLED = 3,
    GROUP_ALREADY_INSTALLED = 4,
} IPC_ERROR;



#endif
