#/bin/sh
echo "ipc modules : "
echo -e "\t $(lsmod | grep ipc) " 

echo "ipc classes on /sys/class/: "
echo -e "\t $(ls /sys/class/  | grep ipc) " 