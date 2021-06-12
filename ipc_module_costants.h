#pragma once

// Kernel-space only
#include <linux/types.h>
#include <linux/ioctl.h>

#define IPC_MAX_GROUPS              16
#define IPC_MSG_SIZE                32
#define IPC_DEV_NAMESIZE            32

#define IPC_ROOT_DEV_NAME           "aosv_ipc_root"
#define IPC_CLASS_NAME              "aosv_ipc_class"

#define IPC_IOCTL_MAGIC             'W'
#define IPC_GROUP_INSTALL           _IO( IPC_IOCTL_MAGIC, 0)
#define IPC_GROUP_UNINSTALL         _IO( IPC_IOCTL_MAGIC, 1)
#define SET_SEND_DELAY              _IO( IPC_IOCTL_MAGIC, 3)
#define REVOKE_DELAYED_MESSAGES     _IO( IPC_IOCTL_MAGIC, 4)
#define FLUSH_DELAYED_MESSAGES      _IO( IPC_IOCTL_MAGIC, 5)
#define SLEEP_ON_BARRIER            _IO( IPC_IOCTL_MAGIC, 6)
#define AWAKE_BARRIER               _IO( IPC_IOCTL_MAGIC, 7)

typedef unsigned int group_t;

typedef enum {
    SUCCESS = 0,
    CANNOT_OPEN_GROUP_ROOT = 1+128,
    INVALID_GROUP_NUM = 2+128,
    GROUP_NOT_INSTALLED = 3+128,
    GROUP_ALREADY_INSTALLED = 4+128,
    GROUP_NOT_EMPTY = 5+128,
    MEM_ALLOCATION_FAILED = 6+128,
    CANNOT_OPEN_GROUP = 7+128,
    NO_MESSAGES = 8+128,
    TIMER_DEL_FAILED = 9+128,
    TIMER_ADD_FAILED = 10+128,
    INVALID_IOCTL_NUM = 11+128,
    GROUP_NOT_OPEN = 13+128,
    GROUP_CLOSING = 14+128,
} IPC_ERROR;

