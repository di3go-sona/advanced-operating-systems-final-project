CURRENT_PATH = $(shell pwd)
LINUX_KERNEL = $(shell uname -r)

RULES_FILENAME = 71-aosv-ipc.rules
MODULE_FILENAME = ipc_module


LINUX_KERNEL_PATH = /lib/modules/$(LINUX_KERNEL)/build/

obj-m += ipc_module.o 
ipc_module-objs :=  ipc_module_main.o ipc_group_root.o ipc_group.o

CFLAGS+="-DKERNELSPACE"

all:
	make -C  $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules		

test:  ipc_lib.c
	gcc  -o test ipc_lib.c test.c -pthread

run:
	/bin/sh $(CURRENT_PATH)/test
	

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean

install:
	cp $(CURRENT_PATH)/$(RULES_FILENAME) /etc/udev/rules.d/
	sudo udevadm control --reload-rules && udevadm trigger

uninstall:
	rm /etc/udev/rules.d/$(RULES_FILENAME)
	sudo udevadm control --reload-rules && udevadm trigger

insert:
	insmod $(MODULE_FILENAME).ko

remove:
	rmmod $(MODULE_FILENAME)

reinsert:
	rmmod $(MODULE_FILENAME)
	insmod $(MODULE_FILENAME).ko
	