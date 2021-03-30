CURRENT_PATH = $(shell pwd)
LINUX_KERNEL = $(shell uname -r)

RULES_FILENAME = $(CURRENT_PATH)/71-aosv-ipc.rules
MODULE_FILENAME = ipc_module


LINUX_KERNEL_PATH = /lib/modules/$(LINUX_KERNEL)/build/

obj-m += $(MODULE_FILENAME).o

all:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules		

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean

install:
	cp $(RULES_FILENAME) /etc/udev/rules.d/
	sudo udevadm control --reload-rules && udevadm trigger

uninstall:
	rm /etc/udev/rules.d/(RULES_FILENAME)
	sudo udevadm control --reload-rules && udevadm trigger

insert:
	insmod $(MODULE_FILENAME).ko

remove:
	rmmod $(MODULE_FILENAME)