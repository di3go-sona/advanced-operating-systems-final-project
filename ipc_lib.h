#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>

#include "ipc_module_costants.h"
#include "ipc_user_macros.h"






void print_code(int err);
int install_group(group_t groupno, char* group_path, int lrn );
int uninstall_group(group_t groupno);

int open_group(group_t groupno);
int close_group(group_t groupno);
int send_msg(group_t groupno, char* payload, ssize_t payload_len);
int recv_msg(group_t groupno, char* payload, ssize_t payload_len);
int set_send_delay(group_t groupno, int delay);
int flush_delayed_messages(group_t groupno);
int revoke_delayed_messages(group_t groupno);
int sleep_on_barrier(group_t groupno);
int awake_barrier(group_t groupno);