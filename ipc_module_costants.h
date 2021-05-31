#pragma once

// Kernel-space only
#include <linux/types.h>
#include <linux/ioctl.h>

#define IPC_MAX_GROUPS 16
#define IPC_MSG_SIZE 32

#define IPC_DEV_NAMESIZE 32
#define IPC_ROOT_DEV_NAME "aosv_ipc_root"
#define IPC_CLASS_NAME "aosv_ipc_class"

#define IPC_IOCTL_MAGIC 'W'

#define IPC_GROUP_INSTALL _IO( IPC_IOCTL_MAGIC, 0)
#define IPC_GROUP_UNINSTALL _IO( IPC_IOCTL_MAGIC, 1)
#define SET_SEND_DELAY _IO( IPC_IOCTL_MAGIC, 3)
#define REVOKE_DELAYED_MESSAGES _IO( IPC_IOCTL_MAGIC, 4)
#define FLUSH_DELAYED_MESSAGES _IO( IPC_IOCTL_MAGIC, 5)

typedef unsigned int group_t;
typedef enum {
    SUCCESS = 0,
    CANNOT_OPEN_GROUP_ROOT = 1,
    INVALID_GROUP_NUM = 2,
    GROUP_NOT_INSTALLED = 3,
    GROUP_ALREADY_INSTALLED = 4,
    GROUP_NOT_EMPTY = 5,
    MEM_ALLOCATION_FAILED = 6,
    CANNOT_OPEN_GROUP = 7,
    NO_MESSAGES = 8,
    TIMER_DEL_FAILED = 9,
    TIMER_ADD_FAILED = 10,
    INVALID_IOCTL_NUM = 11,
} IPC_ERROR;

