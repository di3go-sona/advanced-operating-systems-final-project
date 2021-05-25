#ifndef IPC_COSTANTS_H
#define IPC_COSTANTS_H

// Kernel-space only
#include "linux/types.h"


#define IPC_MAX_GROUPS 16
#define IPC_MSG_SIZE 32

#define IPC_DEV_NAMESIZE 32
#define IPC_ROOT_DEV_NAME "aosv_ipc_root"
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
    GROUP_NOT_EMPTY = 5,
    MEM_ALLOCATION_FAILED = 6
} IPC_ERROR;



#endif
