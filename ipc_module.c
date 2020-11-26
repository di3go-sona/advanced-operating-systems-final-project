#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diego Sonaglia");
MODULE_DESCRIPTION("An ipc message passing/synching module");
MODULE_VERSION("0.01");

static int __init ipc_init(void) {
 printk(KERN_INFO "ipc init");
 return 0;
}

static void __exit ipc_exit(void) {
 printk(KERN_INFO "ipc exit");
}

module_init(ipc_init);
module_exit(ipc_exit);
