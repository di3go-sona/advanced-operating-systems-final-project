#include <stddef.h>
#include <time.h>
#include <string.h>
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

int  _rand_int(int min, int max){
    return (rand() % (max - min + 1)) + min;
}

void _set_timespec(struct timespec *ts, unsigned long ms)
{
    ts->tv_sec = ms / 1000;
    ts->tv_nsec = (ms % 1000) * 1000000;
    // DEBUG("setting timespec to %ld s and %ld ms", ts->tv_sec, ts->tv_nsec / 1000000);
}

int  _send_random_msg(group_t groupno, ssize_t payload_len){
    char buf[payload_len];
    _rand_string(buf, payload_len);
    send_msg(groupno, buf, payload_len);
}

struct thread_args {
    group_t groupno;
    int msg_len;
    int msg_send_delay;
    int repeat_count;
    int repeat_delay;
};

void *sleeper_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(sleep_on_barrier( args -> groupno));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *awaker_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(awake_barrier( args -> groupno));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *sender_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(send_msg( args -> groupno, "hola", 4));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *receiver_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    char buf[128] = {0};
    int i, err;
    
    while ( 1 ) {
        err = recv_msg( args -> groupno, buf, 128);
        if (err != 0) (args -> repeat_count)--;
        if (args -> repeat_count == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *flusher_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(flush_delayed_messages( args -> groupno));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *revoker_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(revoke_delayed_messages( args -> groupno));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

void *delay_setter_thread(void *argp)
{
    struct thread_args* args = (struct thread_args*)argp;  
    struct timespec time, remaining_time;
    int i;
    
    while ( 1 ) {
        print_code(set_send_delay( args -> groupno, args -> msg_send_delay));

        if (--(args -> repeat_count) == 0) break ;
        _set_timespec(&time, args -> repeat_delay);
        nanosleep(&time, &remaining_time);
    }
    return NULL;
}

// int test_install_single(group_t groupno){
//     TEST_BEGIN("test_install_single");

//     install_group(groupno, NULL, 0);
//     uninstall_group(groupno);

//     TEST_END();
// }

// int test_install_random(){
//     TEST_BEGIN("test_install_random");
//     int i;

//     for(i=0; i<20; i++){
//         int r = rand() % IPC_MAX_GROUPS;
//         if (i%2){
//             install_group((group_t) r , NULL, 0);
//         } else {
//             uninstall_group((group_t) r );
//         }
       
//     }
//     for(i=0; i<IPC_MAX_GROUPS; i++){
//         uninstall_group((group_t) i );
//     }

//     TEST_END();
// }

// int test_messaging_single(group_t groupno){
//     TEST_BEGIN("test_messaging_single");
//     int res, i;
//     char msg_buf[32] = {0};

//     res = install_group(groupno, group_path, IPC_GROUP_PATH_SIZE);
//     if (res < 0) return res;
//     sleep(1);
//     for (i=0; i< 32; i++){
//         _send_random_msg(group_path, 16);
//         recv_msg(group_path, msg_buf, 4);
//     }
//     uninstall_group(groupno);
//     TEST_END();
// }

// int test_messaging_multi(group_t groupno, int n_senders, int n_receivers, int msg_num){
//     TEST_BEGIN("test_messaging_multi");
//     int res, i;
//     pthread_t thread_ids[n_senders + n_receivers];
//     struct thread_args args;

//     args.msg_len = 16;
//     args.groupno = groupno;
//     args.max_count = 10;

//     res = install_group(3, NULL, 32);
//     if (res < 0) return res;
//     sleep(1);

//     for (i=0; i< n_senders; i++){
//         pthread_create(thread_ids + i, NULL, sender_thread, &args);
//     }

//     for (; i< n_receivers*2; i++){
//         pthread_create(thread_ids + i, NULL, receiver_thread, &args);
//     }

//     for (i=0; i< n_senders + n_receivers; i++){
//         pthread_join(thread_ids[i], NULL);
//     }

//     uninstall_group(3);
//     TEST_END();
// }

// int test_revoke_single(group_t groupno){
//     TEST_BEGIN("test_revoke_single");
//     int res, i;
//     char group_path[32] = {0};
//     char msg_buf[32] = {0};

//     res = install_group(groupno, group_path, 32);
//     if (res < 0) return res;
//     sleep(1);
//     set_send_delay(group_path, 10);
    
//     for (i=0; i< 4; i++){
//         _send_random_msg(group_path, 4);
//     }

//     revoke_delayed_messages(group_path);
//     uninstall_group(groupno);
//     TEST_END();
// }

// int test_barrier(group_t groupno, int n_waiters, int n_wakers, int min_sleep, int max_sleep, int count){
//     TEST_BEGIN("test_wait");
//     int res, i, group_fd;
    
//     pthread_t thread_ids[n_waiters];
//     const struct timespec time, remaining_time;
//     struct thread_args args;



//     res = install_group(groupno, group_path_buf, 32);
//     if (res < 0) return res ;
//     sleep(1);
//     DEBUG("opening group");
//     group_fd = open(group_path_buf, O_RDWR);
    
//     args.group_fd = group_fd;
//     args.max_count = count;
//     args.min_sleep = min_sleep;
//     args.max_sleep = max_sleep;
//     args.stop = 0;

//     for (i=0; i< n_waiters; i++){
//         pthread_create(thread_ids + i, NULL, sleeper_thread, &args);
//     }

//     for (i=0; i< n_waiters; i++){
//         pthread_create(thread_ids + i, NULL, waker_thread, &args);
//     }

//     _set_timespec(&time, max_sleep*count);
//     nanosleep(&time, &remaining_time);
//     args.stop = 1;
//     ioctl(group_fd, AWAKE_BARRIER);

//     DEBUG("waiting threads");
//     for (i=0; i< n_waiters; i++){
//         pthread_join(thread_ids[i], NULL);
//     }


//     close(group_fd);
//     TEST_END();
// }

int _parse_flags(int argc, char const *argv[], struct thread_args *t_args){
    int i, delay= 100, count=1, group=1;
    for (i=0; i < argc; i++){
        sscanf(argv[i], "--repeat=%d", &count);
        sscanf(argv[i], "--delay=%d", &delay);\
        sscanf(argv[i], "--group=%d", &group);
    }
    t_args -> repeat_delay = delay;
    t_args -> repeat_count = count;
    t_args -> groupno = (group_t) group;
    DEBUG("delay: %dms, count: %d, group: %d", delay, count, group);

}

int main(int argc, char const *argv[])
{
    group_t groupno;
    pthread_t t_id;
    struct thread_args t_args;

    if (argc <= 1){
        printf("Usage: test < sleep | awake | send | recv | flush | revoke | set_delay >  \n");
        return -1;
    } 

    _parse_flags(argc, argv, &t_args);

    print_code(install_group(t_args.groupno, NULL, 0));  
    print_code(open_group(t_args.groupno));  

    if (!strcmp(argv[1], "sleep")){
        pthread_create(&t_id, NULL, sleeper_thread, &t_args);
    } else if (!strcmp(argv[1], "awake")){
        pthread_create(&t_id, NULL, awaker_thread, &t_args);
    } else if (!strcmp(argv[1], "send")){
        pthread_create(&t_id, NULL, sender_thread, &t_args);
    } else if (!strcmp(argv[1], "recv")){
        pthread_create(&t_id, NULL, receiver_thread, &t_args);
    } else if (!strcmp(argv[1], "flush")){
        pthread_create(&t_id, NULL, flusher_thread, &t_args);
    } else if (!strcmp(argv[1], "revoke")){
        pthread_create(&t_id, NULL, revoker_thread, &t_args);
    } else if (!strcmp(argv[1], "set_delay")){
        if (argc <= 1){
        printf("Usage: test set_delay <delay_value>  \n");
        return -1;
    } 
        t_args.msg_send_delay = atoi(argv[2]);
        pthread_create(&t_id, NULL, delay_setter_thread, &t_args);
    }



    pthread_join(t_id, NULL);

    return 0;
}


