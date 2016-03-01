#ifndef _HTTP_H
#define _HTTP_H

#include <string.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "rio.h"

typedef struct mime_type_s{
	const char *type;
	const char *value;
}mime_type_t;

const char *getFileType(const char *type);
void doRequest(int acceptfd);
void readRequestBody(rio_t *rio);
void parseUri(char *uri,char *filename, char *querystring);
void serveStatic(int fd, char *filename, int filesize);
void doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

#endif 