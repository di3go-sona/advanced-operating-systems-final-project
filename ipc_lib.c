#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ipc_lib.h"

int install_group(group_t groupno){ 
    int fd = open("/dev/synch/group", O_RDONLY);
    int ret = ioctl(fd, IPC_GROUP_INSTALL, groupno);
    printf("Installing group %d, res %d\n", groupno, ret);
    return ret;
}

int uninstall_group(group_t groupno){ 
    int fd = open("/dev/synch/group", O_RDONLY);
    int ret = ioctl(fd, IPC_GROUP_UNINSTALL, groupno);
    printf("Uninstalling group %d, res %d\n", groupno, ret );
    return ret;
}


int test_install_sequential(){
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        install_group((group_t) i );
    }
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        uninstall_group((group_t) i );
    }
}

int test_install_single(){
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        install_group((group_t) 1 );
    }
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        uninstall_group((group_t) 1 );
    }
}


int main(){
    test_install_single();
}
