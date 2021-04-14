#include "ipc_module_costants.h"
#include "linux/types.h"

struct class*  	group_dev_class ;
struct cdev* 	group_root_cdev ;
struct cdev* 	group_cdevs[IPC_MAX_GROUPS+1];
dev_t           group_root_devno;