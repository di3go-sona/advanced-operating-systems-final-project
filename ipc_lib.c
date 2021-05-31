#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ipc_lib.h"






void print_code(int err){
    switch (err)
    {
    case SUCCESS:
        OK("success");
        break;
    case CANNOT_OPEN_GROUP_ROOT:
        ERR("cannot open group_root");
        break;
    case INVALID_GROUP_NUM:
        ERR("invalid group num");
        break;
    case GROUP_ALREADY_INSTALLED:
        ERR("group already installed");
        break;
    case GROUP_NOT_INSTALLED:
        ERR("cannot uninstall group not found");
        break;
    case GROUP_NOT_EMPTY:
        ERR("group is not empty");
        break;
    default:
        ERR("unknown code %d",err );
        break;
    }
}

int install_group(group_t groupno, char* group_path, int lrn ){ 
    DEBUG("installing group %d", groupno);
    int fd, res;

    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return -CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_INSTALL, groupno);
    close(fd);

    if (res == GROUP_ALREADY_INSTALLED ){
        DEBUG("group %d already installed", groupno);
    } else if (res == SUCCESS ) {
        DEBUG("group %d installed successfully", groupno);
    } else {
        DEBUG("group %d installation error", groupno);
        return errno;
    }
    

    if (group_path != NULL && lrn > 0){
        snprintf(group_path,lrn, "/dev/synch/group%d", groupno );
    }

    return SUCCESS;    
}

int uninstall_group(group_t groupno){
    DEBUG("uninstalling group %d", groupno);
    int fd, res;


    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return -CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_UNINSTALL, groupno);
    close(fd);

    if (res != 0) {
        DEBUG("failed to uninstall group %d", groupno);
        return errno;
    }

    DEBUG("group %d uninstalled successfully", groupno);
    return SUCCESS;
}

int send_msg(char* group_path, char* payload, ssize_t payload_len){
    DEBUG("sending msg \"%.*s\" to %s", (int)payload_len, payload, group_path);
    int group_fd, res ;
    group_fd = open(group_path, O_WRONLY);
    if (group_fd < 0 ) return -CANNOT_OPEN_GROUP;
    res = write(group_fd, payload, payload_len);
    close(group_fd);
    if (res < 0) return errno;
    return SUCCESS;
}

int recv_msg(char* group_path, char* payload, ssize_t payload_len){
    DEBUG("reading msg from %s", group_path);
    int group_fd, res ;
    group_fd = open(group_path, O_RDONLY);
    if (group_fd < 0 ) return -CANNOT_OPEN_GROUP;
    res = read(group_fd, payload, payload_len);
    close(group_fd);
    if (res < 0) {
        DEBUG("read fail");
        return errno;
    } else {
        DEBUG("succesfully read \"%.*s\"",(int) payload_len, payload );
        return SUCCESS;
    }
    
}


int set_send_delay(char* group_path, int delay){
    int group_fd, res ;
    group_fd = open(group_path, O_RDWR);
    DEBUG("set send delay of %s to %d seconds (fd:%d)", group_path, delay, group_fd);

    if (group_fd < 0 ) return -CANNOT_OPEN_GROUP;
    res = ioctl(group_fd, SET_SEND_DELAY, 10);
    print_code(res);
    print_code(errno);
    printf( "%s\n", strerror(errno) );
    close(group_fd);
    return SUCCESS;
}

int flush_delayed_messages(char* group_path){
    DEBUG("flush delayed messages of %s", group_path);
    int group_fd, res ;
    group_fd = open(group_path, O_RDONLY);
    if (group_fd < 0 ) return -CANNOT_OPEN_GROUP;
    res = ioctl(group_fd, FLUSH_DELAYED_MESSAGES, NULL);
    close(group_fd);
    return SUCCESS;
}

int revoke_delayed_messages(char* group_path){
    DEBUG("revoke delayed messages of %s", group_path);
    int group_fd, res ;
    group_fd = open(group_path, O_RDONLY);
    if (group_fd < 0 ) return -CANNOT_OPEN_GROUP;
    res = ioctl(group_fd, REVOKE_DELAYED_MESSAGES, NULL);
    close(group_fd);
    return SUCCESS;
}
