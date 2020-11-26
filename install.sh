#!/bin/sh

# copying ipcdev rules to the udev rules folder
#cp ./71-ipcdev.rules /etc/udev/rules.d/

sudo insmod ipc_module.ko
sudo rmmod ipc_module
