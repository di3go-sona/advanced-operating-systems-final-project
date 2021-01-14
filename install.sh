#!/bin/sh

# copying ipcdev rules to the udev rules folder
INSTALL_FOLDER=/etc/ipc_group_dev/

make
sudo mkdir $INSTALL_FOLDER
sudo cp -r $PWD/udev_scripts $INSTALL_FOLDER
sudo cp ./71-ipcdev.rules /etc/udev/rules.d/
sudo udevadm control --reload-rules

# make clean
