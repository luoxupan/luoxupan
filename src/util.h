#ifndef _UTIL_H
#define _UTIL_H

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <stdlib.h>


#include "dbg.h"

#define LISTENQ 1024

#define BUFLEN      8192

#define DELIM       "="

#define YG_CONF_OK      0
#define YG_CONF_ERROR   100

typedef struct yg_conf_s {
    void *root;
    int port;
    int thread_num;
}yg_conf_t;


int startServer(int port);
int makeSocketNoBlocking(int fd);
int readConf(char *filename, yg_conf_t *cf, char *buf, int len);

#endif



