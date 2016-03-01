#include "util.h"

int main(){

	int sockfd;
	socklen_t acceptlen;
	struct sockaddr_in clientaddr;
	if((sockfd = startServer(3000)) == -1)
		return -1;


	while(1){
		logInfo("read to accept");
		int acceptfd = accept(sockfd, (struct sockaddr*)&clientaddr, &acceptlen);
		if(acceptfd == -1){
			if((errno == EAGAIN) || (errno ==EWOULDBLOCK)){

				continue;
			}else {
				logErr("accept");
				break;
			}
		}

		doRequest(acceptfd);
		close(acceptfd);
	}

	return 0;
}

