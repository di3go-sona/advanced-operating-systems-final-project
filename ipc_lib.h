#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <pthread.h>

#include "ipc_module_costants.h"




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


int install_group(group_t groupno, char* group_path, int lrn );
int uninstall_group(group_t groupno);
int send_msg(char* group_path, char* payload, ssize_t payload_len);
int recv_msg(char* group_path, char* payload, ssize_t payload_len);

void print_code(int err);
int set_send_delay(char* group_path, int delay);
int flush_delayed_messages(char* group_path);
int revoke_delayed_messages(char* group_path);