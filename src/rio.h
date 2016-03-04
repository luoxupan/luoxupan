#ifndef _RIO_H
#define _RIO_H

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "dbg.h"

#define RIO_BUFSIZE 8192

typedef struct {
	int rio_fd;
	int rio_cnt;
	char *rio_bufptr;
	char rio_buf[RIO_BUFSIZE];
} rio_t;

ssize_t rioReadn(int fd, void *usrbuf, size_t n);
ssize_t rioWriten(int fd, void *usrbuf, size_t n);
void rioReadinitb(rio_t *rp, int fd);
ssize_t rioReadnb(rio_t *rp, void *usrbuf, size_t n);
ssize_t rioReadlineb(rio_t *rp, void *usrbuf, size_t maxlen);

#endif