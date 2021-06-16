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

#define IPC_ERROR_BASE 1024

typedef enum {
    SUCCESS = 0,
    CANNOT_OPEN_GROUP_ROOT =    IPC_ERROR_BASE + 1,
    INVALID_GROUP_NUM =         IPC_ERROR_BASE + 2,
    GROUP_NOT_INSTALLED =       IPC_ERROR_BASE + 3,
    GROUP_ALREADY_INSTALLED =   IPC_ERROR_BASE + 4,
    GROUP_NOT_EMPTY =           IPC_ERROR_BASE + 5,
    MEM_ALLOCATION_FAILED =     IPC_ERROR_BASE + 6,
    CANNOT_OPEN_GROUP =         IPC_ERROR_BASE + 7,
    NO_MESSAGES =               IPC_ERROR_BASE + 8,
    TIMER_DEL_FAILED =          IPC_ERROR_BASE + 9,
    TIMER_ADD_FAILED =          IPC_ERROR_BASE + 10,
    INVALID_IOCTL_NUM =         IPC_ERROR_BASE + 11,
    GROUP_NOT_OPEN =            IPC_ERROR_BASE + 13,
    GROUP_CLOSING =             IPC_ERROR_BASE + 14
} IPC_ERROR;

