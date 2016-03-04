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

	if(listen(sockfd, LISTENQ) < 0)
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



int readConf(char *filename, yg_conf_t *cf, char *buf, int len){
	FILE *fp = fopen(filename, "r");
	if(!fp){
		logErr("cannot open config file: %s", filename);
		return YG_CONF_ERROR;
	}

	int pos = 0;
	char *delim_pos;
	int line_len;
	char *cur_pos = buf+pos;

	while(fgets(cur_pos, len-pos, fp)){
		delim_pos = strstr(cur_pos, DELIM);
		line_len = strlen(cur_pos);

		if(!delim_pos)
			return YG_CONF_ERROR;

		if(cur_pos[strlen(cur_pos) - 1] == '\n'){
			cur_pos[strlen(cur_pos) - 1] = '\0';
		}

		if(strncmp("root", cur_pos, 4) == 0){
			cf->root = delim_pos + 1;
		}

		if(strncmp("port", cur_pos, 4) == 0){
			cf->port = atoi(delim_pos + 1);
		}

		if(strncmp("threadnum", cur_pos, 9) == 0){
			cf->thread_num = atoi(delim_pos + 1);
		}

		cur_pos += line_len;
	}
	fclose(fp);
	return YG_CONF_OK;
}

