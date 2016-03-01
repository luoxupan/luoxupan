#include "rio.h"


ssize_t rioReadn(int fd, void *usrbuf, size_t n) {
	char *buf = (char *)usrbuf;
	size_t len = n;
	ssize_t readlen;
	while(len){
		if((readlen = read(fd,buf,len)) < 0 ) {
			if(errno == EINTR)
				readlen = 0;
			else
				return -1;
		} else if(readlen == 0)
			break;
		len -= readlen;
		buf +=readlen;
	}
	return (n - len);
}

ssize_t rioWriten(int fd, void *usrbuf, size_t n){

	ssize_t writenlen;
	size_t len = n;
	char *buf = (char *)usrbuf;
	while(len){
		if((writenlen = write(fd, buf, len)) <= 0){
			if(errno == EINTR)
				writenlen = 0;
			else
				return -1;
		}
		len -= writenlen;
		buf += writenlen;
	}

	return n;
}

static ssize_t rioRead(rio_t *rp, char *usrbuf, size_t n){
	int cnt;
	while(rp->rio_cnt <= 0){
		rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
		if(rp->rio_cnt < 0){
			if(errno != EINTR)
				return -1;
		} 
		else if(rp->rio_cnt == 0)
			return 0;
		else
			rp->rio_bufptr = rp->rio_buf;
	}
	cnt = n;
	if(rp->rio_cnt < cnt)
		cnt = rp->rio_cnt;
	memcpy(usrbuf, rp->rio_bufptr, cnt);
	rp->rio_bufptr += cnt;
	rp->rio_cnt -= cnt;
	return cnt;
}



void rioReadinitb(rio_t *rp, int fd){
	rp->rio_fd = fd;
	rp->rio_cnt = 0;
	rp->rio_bufptr = rp->rio_buf;
}


ssize_t rioReadnb(rio_t *rp, void *usrbuf, size_t n){
	size_t len = n;
	ssize_t readlen;
	char *bufp = (char *)usrbuf;
	while(len){
		if((readlen = rioRead(rp,bufp,len)) < 0){
			if(errno == EINTR)
				readlen = 0;
			else
				return -1;
		}
		else if(readlen == 0)
			break;
		len -= readlen;
		bufp += readlen;
	}
	return (n - len);
}


ssize_t rioReadlineb(rio_t *rp, void *usrbuf, size_t maxlen){
	int n, rc;
	char ch, *bufp = (char *)usrbuf;

	for(n = 1; n < maxlen; n++){
		if((rc = rioRead(rp, &ch, 1)) == 1){
			*bufp++ = ch;
			if(ch == '\n')
				break;
		}
		else if(rc == 0){
			if(n == 1)
				return 0;
			else 
				break;
		}
		else
			return -1;
	}
	*bufp = 0;
	return n;
}
