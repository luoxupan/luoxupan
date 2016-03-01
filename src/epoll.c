#include "epoll.h"
#include "dbg.h"

struct epoll_event *events;

int yg_epoll_create(int flags){
	int fd = epoll_create1(flags);
	check(fd > 0, "epoll_create1");
	events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAXEVENTS);

	return fd;
}

void yg_epoll_add(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, event);
	check(rc == 0, "epoll_add");
	return;
}

void yg_epoll_mod(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, event);
	check(rc == 0, "epoll_mod")
	return;	
}

void yg_epoll_del(int epfd, int fd, struct epoll_event *event){
	int rc = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, event);
	check(rc == 0, "epoll_del");
	return;
}

int yg_epoll_wait(int epfd, struct epoll_event *events,int maxevents, int timeout){
	int n = epoll_wait(epfd, events, maxevents, timeout);
	check(n >= 0, "epoll_wait");
	return n;
}


