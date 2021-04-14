#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ipc_lib.h"
#include <errno.h>


void print_error(int err){
    switch (err)
    {
    case SUCCESS:
        printf("SUCCESS\n");
        break;
    case CANNOT_OPEN_GROUP_ROOT:
        printf("ERR: cannot open group_root\n");
        break;
    case INVALID_GROUP_NUM:
        printf("ERR: invalid group num\n");
        break;
    default:
        printf("ERR: unknown  error %d\n",err );
        break;
    }
}

int install_group(group_t groupno){ 
    printf("Installing group %d\n", groupno);
    int fd, err;

    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    err = ioctl(fd, IPC_GROUP_INSTALL, groupno);
    close(fd);
    if (err) err = errno;

    return err;
    
}

int uninstall_group(group_t groupno){
    printf("Uninstalling group %d\n", groupno);
    int fd, err;

    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    err = ioctl(fd, IPC_GROUP_UNINSTALL, groupno);
    close(fd);
    if (err) err = errno;

    return err;
}


int test_install_single(){
    printf("### Test install single\n");

    print_error(install_group(3));
    print_error(uninstall_group(3));
}


int test_install_sequential(){
    printf("### Test install sequential\n");

    for(int i=0; i<IPC_MAX_GROUPS; i++){
        print_error(install_group((group_t) i ));
    }
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        print_error(uninstall_group((group_t) i ));
    }
}

int test_install_random(){
    printf("### Test install randpm\n");

    for(int i=0; i<20; i++){
        int r = rand() % IPC_MAX_GROUPS;
        if (i%2){
            print_error(install_group((group_t) r ));
        } else {
            print_error(uninstall_group((group_t) r ));
        }
       
    }
    for(int i=0; i<IPC_MAX_GROUPS; i++){
        print_error(uninstall_group((group_t) i ));
    }
}





int main(){
    test_install_single();
    test_install_sequential();
    test_install_random();
}
