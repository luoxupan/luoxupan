#ifndef _EPOLL_H
#define _EPOLL_H

#include <sys/epoll.h>

#define MAXEVENTS 1024

int yg_epoll_create(int flags);
void yg_epoll_add(int epfd, int fd, struct epoll_event *event);
void yg_epoll_mod(int epfd, int fd, struct epoll_event *event);
void yg_epoll_del(int epfd, int fd, struct epoll_event *event);
int yg_epoll_wait(int epfd, struct epoll_event *event,int maxevents, int timeout);


#endif
