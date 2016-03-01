#include "util.h"
#include "http.h"
#include "epoll.h"

extern struct epoll_event *events;

int main(int argc, char* argv[]){

	int sockfd;
	int rc;
	socklen_t acceptlen;
	struct sockaddr_in clientaddr;
	sockfd = startServer(3000);

	rc = makeSocketNoBlocking(sockfd);
	check(rc == 0, "makeSocketNoBlocking");

	int epfd = yg_epoll_create(0);
	struct epoll_event event;
	event.data.fd = sockfd;
	event.events = EPOLLIN | EPOLLET;
	yg_epoll_add(epfd, sockfd, &event);


	while(1){
		logInfo("read to wait");
		int n;
		n = yg_epoll_wait(epfd, events, MAXEVENTS, -1);

		int i;
		for(i=0; i<n; i++){
			if((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP) ||
                (!(events[i].events & EPOLLIN))){
				logErr("epoll error");
				close(events[i].data.fd);
				continue;
			}
			else if(sockfd == events[i].data.fd){
				while(1){
					logInfo("read to accept");
					int acceptfd = accept(sockfd, (struct sockaddr*)&clientaddr, &acceptlen);
					if(acceptfd == -1){
						if((errno == EAGAIN) || (errno ==EWOULDBLOCK)){

							break;
						}else {
							logErr("accept");
							break;
						}
					}

					doRequest(acceptfd);
					close(acceptfd);
				}
			}
		}
	}


	return 0;
}

