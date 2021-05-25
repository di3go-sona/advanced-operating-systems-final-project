#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "ipc_lib.h"
#include <errno.h>
#include <pthread.h>


#define DEBUG_ENABLED 


#ifdef DEBUG_ENABLED
#define DEBUG(...) do{ fprintf(stderr, "\x1b[30;1m[ DEBUG ][ pid %ld ] ", pthread_self()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\x1b[0m\n");} while( 0 )
#else
#define DEBUG(...) do{ } while ( 0 )
#endif


#define LOG(...) do{ fprintf(stderr, "[ LOG ][ pid %ld ] ", pthread_self()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\n");} while( 0 )
#define ERR(...) do{ fprintf(stderr, "[ ERR ][ pid %ld ] ", pthread_self()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\n");} while( 0 )
#define OK(...) do{ fprintf(stderr, "[ OK ][ pid %ld ] ", pthread_self()); fprintf( stderr, __VA_ARGS__ ); fprintf(stderr, "\n");} while( 0 )


#define TEST_BEGIN(...) do{ printf( "[ TEST_BEGIN ] "); printf(  __VA_ARGS__ ); printf("\n");} while( 0 )
#define TEST_END(...) do{ printf( "[ TEST_END ]\n\n"); } while( 0 )


void rand_string(char *str, size_t size)
{
    size_t n;
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVZXWY";
    
    if (size--) {
        for (n = 0; n < size; n++) {
            int key = rand() % (int) (sizeof charset - 1);
            str[n] = charset[key];
        }
    }
}

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

int install_group(group_t groupno, char* path_buffer, int path_len ){ 
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
    

    if (path_buffer != NULL && path_len > 0){
        snprintf(path_buffer,path_len, "/dev/synch/group%d", groupno );
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

    if (res != 0) return errno;

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

int send_random_msg(char* group_path, ssize_t payload_len){
    char payload[payload_len];
    rand_string(payload, payload_len);
    send_msg(group_path, payload, payload_len);
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





int test_install_single(group_t groupno){
    TEST_BEGIN("test_install_single");

    install_group(groupno, NULL, 0);
    uninstall_group(groupno);

    TEST_END();
}




int test_install_random(){
    TEST_BEGIN("test_install_random");
    int i;

    for(i=0; i<20; i++){
        int r = rand() % IPC_MAX_GROUPS;
        if (i%2){
            install_group((group_t) r , NULL, 0);
        } else {
            uninstall_group((group_t) r );
        }
       
    }
    for(i=0; i<IPC_MAX_GROUPS; i++){
        uninstall_group((group_t) i );
    }

    TEST_END();
}


int test_messaging_single(group_t groupno){
    TEST_BEGIN("test_messaging_single");
    int res, i;
    char group_path[32] = {0};
    char msg_buf[32] = {0};

    res = install_group(groupno, group_path, 32);
    if (res < 0) return res;
    sleep(1);
    for (i=0; i< 32; i++){
        send_random_msg(group_path, 16);
        recv_msg(group_path, msg_buf, 4);
    }
    uninstall_group(groupno);
    TEST_END();
}


struct thread_args {
    char* group_path;
    int len;
    int* counter;
    int max_count;
};

void *receiver_thread(void *argp){
    struct thread_args* args = (struct thread_args*)argp;  
    int i;

    char *buf = malloc(args -> len);
    for (i=0; i< args -> max_count; i++){
        recv_msg(args -> group_path, buf, args ->len);

    }
    free(buf);
    return NULL;
}
void *sender_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    int i;

    for (i=0; i< args -> max_count; i++){
        send_random_msg(args-> group_path, args -> len);

    }
    return NULL;
}



int test_messaging_multi(group_t groupno, int threads_num){
    TEST_BEGIN("test_messaging_multi");
    int res, i;
    char group_path[32] = {0};
    pthread_t thread_ids[threads_num*2];
    struct thread_args args;

    args.len = 16;
    args.group_path = (char* )&group_path;
    args.max_count = 10;

    res = install_group(3, group_path, 32);
    if (res < 0) return res;
    sleep(1);

    for (i=0; i< threads_num; i++){
        pthread_create(thread_ids + i, NULL, sender_thread, &args);
    }

    for (; i< threads_num*2; i++){
        pthread_create(thread_ids + i, NULL, receiver_thread, &args);
    }

    for (i=0; i< threads_num*2; i++){
        pthread_join(thread_ids[i], NULL);
    }

    uninstall_group(3);
    TEST_END();
}


int main(){
    test_install_single(3);
    test_install_random();
    test_messaging_single(3);
    test_messaging_multi(3,6);
}
