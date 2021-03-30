#define IPC_MAX_GROUPS 16
#define IPC_ROOT_DEV_NAME "aosv_ipc_root"
#define IPC_DEV_NAME "aosv_ipc_group"
#define IPC_CLASS_NAME "aosv_ipc_class"

#define IPC_GROUP_INSTALL 0
#define IPC_GROUP_UNINSTALL 1

typedef unsigned int group_t;
extern struct class*  	dev_class ;