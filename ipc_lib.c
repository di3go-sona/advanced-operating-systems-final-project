#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "ipc_lib.h"

#define IPC_GROUP_PATH_SIZE 32

// Please note that this library is not multi-process safe
// because file descriptors are created and stored  once for each call 
// to open_group/close_group ( thus not accounting for fd copies during fork).
// In order to use it in multiprocess environments only call open_group
// from the parent process before forking and close_group after every 
// child process terminate.

#define _check_installed(groupno)  ({ if (groups_path[groupno][0]) {return GROUP_NOT_INSTALLED;} })
#define _check_open(groupno)  ({ _check_installed(groupno); if (groups_fd[groupno] == 0) {return GROUP_NOT_OPEN;} })

int groups_fd[IPC_MAX_GROUPS + 1] = {0};
char groups_path[IPC_GROUP_PATH_SIZE][IPC_MAX_GROUPS + 1] = {0};

void print_code(int err){
    switch (err)
    {
    case SUCCESS:
        DEBUG("Success");
        break;
    case CANNOT_OPEN_GROUP_ROOT:
        ERR("Cannot open group root");
        break;
    case INVALID_GROUP_NUM:
        ERR("invalid group num");
        break;
    case GROUP_ALREADY_INSTALLED:
        ERR("group already installed");
        break;
    case GROUP_NOT_INSTALLED:
        ERR("Group not installed");
        break;
    case GROUP_NOT_OPEN:
        ERR("Group not open");
        break;
    case GROUP_NOT_EMPTY:
        ERR("group is not empty");
        break;
    case NO_MESSAGES:
        WARN("No message found");
        break;
    default:
        ERR("%s",strerror(err) );
        break;
    }
}

int install_group(group_t groupno, char* group_path, int lrn ){ 
    DEBUG("Installing group %d", groupno);
    int fd, res;


    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_INSTALL, groupno);
    close(fd);

    if (res != SUCCESS ) switch (errno)
    {
    case (GROUP_ALREADY_INSTALLED):
        WARN("Group %d already installed", groupno);
        res = SUCCESS;
        break;
    default:
        return errno;
    }

    snprintf(groups_path[groupno],IPC_GROUP_PATH_SIZE, "/dev/synch/group%d", groupno );
    if (group_path != NULL && lrn > 0){
        snprintf(group_path,lrn, "/dev/synch/group%d", groupno );
    }

    while( access( groups_path[groupno], F_OK  ) != 0) {} 

    return SUCCESS;    
}

int uninstall_group(group_t groupno){
    DEBUG("uninstalling group %d", groupno);
    int fd, res;


    fd = open("/dev/synch/group", O_RDONLY);
    if (fd<0) return CANNOT_OPEN_GROUP_ROOT;

    res = ioctl(fd, IPC_GROUP_UNINSTALL, groupno);
    close(fd);

    if (res != 0) {
        DEBUG("failed to uninstall group %d", groupno);
        return errno;
    }

    DEBUG("group %d uninstalled successfully", groupno);
    return SUCCESS;
}

int open_group(group_t groupno){
    DEBUG("Opening group %d at %s ", groupno, groups_path[groupno]);

    int fd = open(groups_path[groupno], O_RDWR);
    if (fd < 0){
        return errno;
    }
    groups_fd[groupno] = fd;
    return SUCCESS;

}

int close_group(group_t groupno){
    int err =  close(groups_fd[groupno]);
    if (err < 0){
        return errno;
    }
    groups_fd[groupno] = 0;
    return SUCCESS;
}

int send_msg(group_t groupno, char* payload, ssize_t payload_len){
    int res;

    DEBUG("sending msg \"%.*s\" to %d", (int)payload_len, payload, groupno);
    res = write(groups_fd[groupno], payload, payload_len);
    if (res < 0) return errno;
    return SUCCESS;
}

int recv_msg(group_t groupno, char* payload, ssize_t payload_len){
    DEBUG("reading msg from %d", groupno);
    int res;
    res = read(groups_fd[groupno], payload, payload_len);
    if (res < 0) {
        DEBUG("read fail");
        return errno;
    } else {
        DEBUG("succesfully read \"%.*s\"",(int) payload_len, payload );
        return SUCCESS;
    }
    
}

int set_send_delay(group_t groupno, int delay){
    int err ;
    DEBUG("set send delay of group %d to %d seconds", groups_fd[groupno], delay );

    err = ioctl(groups_fd[groupno], SET_SEND_DELAY, delay);
    DEBUG( "ioctl: %s\n", strerror(errno) );
    return (err) ? errno : SUCCESS;
}

int flush_delayed_messages(group_t groupno){
    int err ;
    DEBUG("flush delayed messages of group %d", groupno);


    err = ioctl(groups_fd[groupno], FLUSH_DELAYED_MESSAGES, NULL);
    return (err) ? errno : SUCCESS;
}

int revoke_delayed_messages(group_t groupno){
    int err ;
    DEBUG("revoke delayed messages of group %d", groupno);

    err = ioctl(groups_fd[groupno], REVOKE_DELAYED_MESSAGES, NULL);
    return (err) ? errno : SUCCESS;
}

int sleep_on_barrier(group_t groupno){
    int err ;
    DEBUG("sleeping on barrier of group %d", groupno);

    err = ioctl(groups_fd[groupno], SLEEP_ON_BARRIER, NULL);
    return (err) ? errno : SUCCESS;
}

int awake_barrier(group_t groupno){
    int err ;
    DEBUG("awaking barrier of group %d", groupno);

    err = ioctl(groups_fd[groupno], AWAKE_BARRIER, NULL);
    return (err) ? errno : SUCCESS;
}
