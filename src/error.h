#ifndef _ERROR_H
#define _ERROR_H

#include <stdio.h>
#include <netdb.h>

void unixError(char *msg){
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}

void posixError(int code, char *msg){
	fprintf(stderr,"%s: %s\n",msg,strerror(code));
	exit(0);
}

#endif