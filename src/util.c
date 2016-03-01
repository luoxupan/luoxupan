#include "util.h"

int startServer(int port){

	if(port <= 0)
		port = 3000;

	int optval = 1;
	struct sockaddr_in serveraddr;

	int sockfd;
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
		return -1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
			(const void*)&optval, sizeof(int)) < 0 )
		return  -1;

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	if(bind(sockfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	if(listen(sockfd, LISTENSIZE) < 0)
		return -1;

	return sockfd;
}


int makeSocketNoBlocking(int fd){
	int flags, s;
	flags = fcntl(fd, F_GETFL, 0);
	if(flags == -1){
		logErr("fcntl");
		return -1;
	}
	flags |= O_NONBLOCK;
	s = fcntl(fd, F_SETFL, flags);
	if(s == -1){
		logErr("fcntl");
		return -1;
	}

	return 0;
}



