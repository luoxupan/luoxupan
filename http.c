#include "http.h"
#include "rio.h"
#include "dbg.h"

#define MAXLINE 8192
#define SHORTLINE 512

#define root "/home/ygxqqx/server"

mime_type_t ygxqqx_mime[] = {
	{".html", "text/html"},
	{".xml", "text/xml"},
	{".xhtml", "application/xhtml+xml"},
	{".txt", "text/plain"},
	{".rft", "application/rft"},
	{".pdf", "application/pdf"},
	{".word", "application/msword"},
	{".png", "image/png"},
	{".gif", "image/gif"},
	{".jpg", "image/jpeg"},
	{".jpeg", "image/jpeg"},
	{".au", "audio/basic"},
	{".mpeg", "video/mpeg"},
	{".mpg", "video/mpeg"},
	{".avi", "video/x-msvideo"},
	{".gz", "application/x-gzip"},
	{".tar", "application/x-tar"},
	{".css", "text/css"},
	{NULL,"text/plain"},
};

void doRequest(int acceptfd){

	int rc;
	rio_t rio;
	char method[SHORTLINE],uri[SHORTLINE],version[SHORTLINE];
	char buf[MAXLINE];
	char filename[SHORTLINE];
	struct stat sbuf;

	rioReadinitb(&rio, acceptfd);
	rioReadlineb(&rio, buf, MAXLINE);

	sscanf(buf,"%s %s %s",method,uri,version);
	logInfo("req line = %s", buf);

	if(strcasecmp(method, "GET")){
		doError(acceptfd, method, "501", "Not Implemented", "ygxqqx doesn't support");
		return;
	}

	readRequestBody(&rio);
	parseUri(uri, filename, NULL);

	if(stat(filename, &sbuf) < 0){
		doError(acceptfd, filename, "404", "Not Found", "ygxqqx can't find the file");
		return;
	}


	if(!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)){
		doError(acceptfd, filename, "403", "Forbidden", "ygxqqx can't read the file");
		return;
	}

	serveStatic(acceptfd, filename, sbuf.st_size);

}


void doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char header[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n</p>", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny web server</em>\r\n", body);

    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));
    logInfo("header  = \n %s\n", header);
    rioWriten(fd, header, strlen(header));
    rioWriten(fd, body, strlen(body));
    logInfo("leave clienterror\n");
    return;
}


void readRequestBody(rio_t *rio){
	check(rio != NULL, "rio == NULL");
	char buf[MAXLINE];
	rioReadlineb(rio, buf, MAXLINE);
	while(strcmp(buf, "\r\n")){
		logInfo("%s", buf);
		rioReadlineb(rio, buf, MAXLINE);
	}
	return;
}


void parseUri(char *uri,char *filename, char *querystring){
	strcpy(filename, root);
	strcat(filename, uri);

	char *last_comp = rindex(filename, '/');
	char *last_dot = rindex(last_comp, '.');

	if(last_dot == NULL && filename[strlen(filename)-1] != '/'){
		strcat(filename, "/");
	}

	if(uri[strlen(uri)-1]=='/' || filename[strlen(filename)-1]=='/'){
		strcat(filename, "index.html");
	}
	return;
}


void serveStatic(int fd, char *filename, int filesize){
    char header[MAXLINE];
    int n;
    
    const char *file_type;
    const char *dot_pos = rindex(filename, '.');
    file_type = getFileType(dot_pos);

    sprintf(header, "HTTP/1.0 200 OK\r\n");
    sprintf(header, "%sServer: ygxqqx\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n", header, filesize);
    sprintf(header, "%sContent-type: %s\r\n\r\n", header, file_type);
//    sprintf(header, "%sConnection: close\r\n\r\n", header);

    n = rioWriten(fd, header, strlen(header));
    check(n == strlen(header), "rioWriten error");

    int srcfd = open(filename, O_RDONLY, 0);
    check(srcfd > 2, "open error");
    char *srcaddr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    check(srcaddr > 0, "mmap error");
    close(srcfd);

    n = rioWriten(fd, srcaddr, filesize);
    check(n == filesize, "rioWriten error");

    munmap(srcaddr, filesize);
    return;
}


const char *getFileType(const char *type){
	if(type == NULL)
		return "text/plain";
	int i = 0;
	while(ygxqqx_mime[i].type!=NULL){
		if(strcmp(type, ygxqqx_mime[i].type) == 0)
			return ygxqqx_mime[i].value;
		i++;
	}
	return ygxqqx_mime[i].value;
}