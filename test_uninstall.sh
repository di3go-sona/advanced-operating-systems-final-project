#/bin/sh
successful=0
ipc_mod="$(lsmod | grep ipc)"
if ! [[  $ipc_mod =~ "ipc_module" ]]
then
    echo "Cannot uninstall, no module found !"
else
    echo "Ipc module found,  uninstalling "
    sudo rmmod ipc_module
    for i in {0..15} 
    do 
        if ! [[  "$(lsmod | grep ipc)"  =~ "ipc_module" ]]
        then
            echo "Uninstall successful"
            successful=1
            break
        else
            echo -ne "Uninstalling module, $i s elapsed"'\r' 
            sleep 1 
        fi
    done 

    if [[  successful  =  0 ]]
    then
        echo ""
        echo "Uninstall error "
    fi
fi