#include <stddef.h>

#include "ipc_lib.h"


void _rand_string(char *str, size_t size)
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

int _send_random_msg(char* group_path, ssize_t payload_len){
    char payload[payload_len];
    _rand_string(payload, payload_len);
    send_msg(group_path, payload, payload_len);
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
    sleep(1);
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
        _send_random_msg(args-> group_path, args -> len);

    }
    return NULL;
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
        _send_random_msg(group_path, 16);
        recv_msg(group_path, msg_buf, 4);
    }
    uninstall_group(groupno);
    TEST_END();
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
    // test_install_single(3);
    // test_install_random();
    // test_messaging_single(3);
    test_messaging_multi(3,6);
}
