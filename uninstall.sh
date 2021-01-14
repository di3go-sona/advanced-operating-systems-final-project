#!/bin/sh
sudo rm  /etc/udev/rules.d/71-ipcdev.rules
sudo udevadm control --reload-rules
sleep 3s


sudo rm -rf /etc/ipc_group_dev
make clean
