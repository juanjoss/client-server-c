#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <pthread.h>
#include "queue.h"

// std HTTP port
#define SERVER_PORT 8080

// buffer length
#define BUFF_SIZE 4096

// socket error constant
#define SOCKET_ERROR (-1)

// server backlog constant
#define SERVER_BACKLOG 1

// number of threads in the pool
#define THREAD_POOL_SIZE 20

// array of threads
pthread_t thread_pool[THREAD_POOL_SIZE];

// thread function to init thread pool
void* thread_function(void *arg);

// mutex for avoiding queue race conditions
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// condition varaible to make threads wait until a new connection arrives
/* 
    condition variables are used to make threads wait until something happends,
    avoiding busy waiting.
*/
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

// error handling
void err_die(const char *fmt, ...);

// check
void check(int exp, const char *msg);

char *bin2hex(const unsigned char *input, size_t len);

// handle client connections
void *handle_connection(void *p_client_socket);

#endif