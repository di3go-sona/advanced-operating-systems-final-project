#/bin/sh
successful=0
ipc_mod="$(lsmod | grep ipc)"
if [[ $ipc_mod =~ "ipc_module" ]]
then
    echo "Cannot install, found $ipc_mod  !"

else
    echo "No ipc modules found, installing"
    sudo insmod ipc_module.ko
    for i in {0..15} 
    do 
        if [[  "$(lsmod | grep ipc)"  =~ "ipc_module" ]]
        then
            echo "Install successful"
            successful=1
            break
        else
            echo -ne "Installing module, $i s elapsed"'\r' 
            sleep 1 
        fi
    done 

    if [[  successful  =  0 ]]
    then
        echo ""
        echo "Install error "
    fi
fi
