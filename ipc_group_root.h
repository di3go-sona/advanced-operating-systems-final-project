
int ipc_group_root_install(void);
int ipc_group_root_uninstall(void);

extern struct class*  	dev_class ;
extern struct device*	root_device ;
extern struct cdev* 	root_cdev ;
extern dev_t devno;
