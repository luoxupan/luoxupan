#ifndef _UTIL_H
#define _UTIL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <arpa/inet.h>


#include "dbg.h"

#define LISTENSIZE 1024


int startServer(int port);
int makeSocketNoBlocking(int fd);


#endif



