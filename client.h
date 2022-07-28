#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdarg.h>

// std HTTP port
#define SERVER_PORT 8080

// buffer length
#define BUFF_SIZE 4096

// error handling
void err_die(const char *fmt, ...);

#endif