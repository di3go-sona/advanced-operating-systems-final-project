#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diego Sonaglia");
MODULE_DESCRIPTION("An ipc message passing/synching module for the AOSV course @ La sapieza");
MODULE_VERSION("0.01");



#include "ipc_group_root.h"

static int ipc_init(void) {
    printk(KERN_INFO "module init");
    ipc_group_root_install();
    return 0;
}

static void  ipc_exit(void) {
	printk(KERN_INFO "module exit");
	ipc_group_root_uninstall();
}


module_init(ipc_init);
module_exit(ipc_exit);
