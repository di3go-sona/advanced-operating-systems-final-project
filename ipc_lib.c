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
    case GROUP_ALREADY_INSTALLED:
        printf("ERR: group already installed\n");
        break;
    case GROUP_NOT_INSTALLED:
        printf("ERR: group not installed\n");
        break;
    default:
        printf("ERR: unknown  error %d\n",err );
        break;
    }
}

int install_group(group_t groupno, char* path_buffer, int path_len ){ 
    printf("Installing group %d\n", groupno);
    int fd, err, res;
    err = SUCCESS;

    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_INSTALL, groupno);
    close(fd);

    if (res != 0) err = errno;
    
    if (!(err == SUCCESS || err == GROUP_ALREADY_INSTALLED )) {
        return errno;
    } else {
        if (path_buffer != NULL && path_len > 0){
            snprintf(path_buffer,path_len, "/dev/synch/group%d", groupno );
            return SUCCESS;
        }
    }
}

int uninstall_group(group_t groupno){
    printf("Uninstalling group %d\n", groupno);
    int fd, err, res;
    err = SUCCESS;


    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_UNINSTALL, groupno);
    close(fd);

    if (res != 0) err = errno;

    return err;
}


int test_install_single(){
    printf("### Test install single\n");

    print_error(install_group(3, NULL, 0));
    print_error(uninstall_group(3));
}


int test_install_sequential(){
    int i;
    printf("### Test install sequential\n");
    

    for(i=0; i<IPC_MAX_GROUPS; i++){
        print_error(install_group((group_t) i, NULL, 0 ));
    }
    for(i=0; i<IPC_MAX_GROUPS; i++){
        print_error(uninstall_group((group_t) i ));
    }
}

int test_install_random(){
    int i;
    printf("### Test install randpm\n");

    for(i=0; i<20; i++){
        int r = rand() % IPC_MAX_GROUPS;
        if (i%2){
            print_error(install_group((group_t) r , NULL, 0));
        } else {
            print_error(uninstall_group((group_t) r ));
        }
       
    }
    for(i=0; i<IPC_MAX_GROUPS; i++){
        print_error(uninstall_group((group_t) i ));
    }
}


int test_messaging_single(void){
    printf("[TEST] test_messaging_single \n");
    int res, i, fd;
    char group_path[32] = {0};
    char msg_buf[32] = {0};

    res = install_group(3, group_path, 32);
    printf("group_install res: %d, path: %s\n", res, group_path);
    if (!(res < 0)){
        fd = open(group_path, O_RDWR);
        printf("fd: %d\n", fd);
        for (i=0; i< 32; i++){
            printf("Loop %d", i);
            sleep(1);
            read(fd, msg_buf, 1);
            write(fd, "hola", 1);
            printf("\n");
        }
    }
    uninstall_group(3);

}



int main(){
    // test_install_single();
    // test_install_sequential();
    // test_install_random();
    test_messaging_single();
}
