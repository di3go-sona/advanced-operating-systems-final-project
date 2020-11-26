CURRENT_PATH = $(shell pwd)
LINUX_KERNEL = $(shell uname -r)
LINUX_KERNEL_PATH = /lib/modules/$(LINUX_KERNEL)/build/

obj-m += ipc_module.o

all:
			make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

clean:
			make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean

