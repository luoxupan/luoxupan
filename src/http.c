#include <strings.h>
#include "http.h"

static const char* getFileType(const char *type);
static void parseUri(char *uri, int length, char *filename, char *querystring);
static void doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
static char *ROOT = NULL;

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

void doRequest(void *ptr){
	yg_http_request_t *r = (yg_http_request_t *)ptr;
	int fd = r->fd;

	int rc;
	char filename[SHORTLINE];
	struct stat sbuf;
	int n;
	ROOT = r->root;
	deBug("ROOT=%s", ROOT);

	for(;;){
		n = read(fd, r->last, (uint64_t)r->buf + MAX_BUF - (uint64_t)r->last);
		check((uint64_t)r->buf + MAX_BUF > (uint64_t)r->last, "(uint64_t)r->buf + MAX_BUF");

		if(n == 0){
			logInfo("read return 0, ready to close fd %d", fd);
			goto err;
		}

		if(n < 0){
			if (errno != EAGAIN) {
                logErr("read err, and errno = %d", errno);
                goto err;
            }
            break;
		}

		r->last += n;
		check(r->last <= r->buf + MAX_BUF, "r->last <= MAX_BUF");
		logInfo("ready to parse request line");
		rc = yg_http_parse_request_line(r);
		if (rc == YG_AGAIN) {
            continue;
        } else if (rc != YG_OK) {
            logErr("rc != YG_OK");
            goto err;
        }

        logInfo("method == %.*s",r->method_end - r->request_start, r->request_start);
        logInfo("uri == %.*s", r->uri_end - r->uri_start, r->uri_start);

        logInfo("ready to parse request body");
        rc  = yg_http_parse_request_body(r);
        if (rc == YG_AGAIN) {
            continue;
        } else if (rc != YG_OK) {
            logErr("rc != YG_OK");
            goto err;
        }

        yg_http_out_t *out = (yg_http_out_t *)malloc(sizeof(yg_http_out_t));
        if (out == NULL) {
            logErr("no enough space for yg_http_out_t");
            exit(1);
        }

        rc = yg_init_out_t(out, fd);
        check(rc == YG_OK, "yg_init_out_t");

        parseUri(r->uri_start, r->uri_end - r->uri_start, filename, NULL);

        if(stat(filename, &sbuf) < 0) {
            doError(fd, filename, "404", "Not Found", "ygxqqx can't find the file");
            continue;
        }

        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            doError(fd, filename, "403", "Forbidden",
                    "ygxqqx can't read the file");
            continue;
        }

        out->mtime = sbuf.st_mtime;

        yg_http_handle_header(r, out);
        check(list_empty(&(r->list)) == 1, "header list should be empty");

        if (out->status == 0) {
            out->status = YG_HTTP_OK;
        }

        serveStatic(fd, filename, sbuf.st_size, out);

        free(out);
        if (!out->keep_alive) {
            logInfo("no keep_alive! ready to close");
            goto close;
        }


	}
	return;

err:
close:
	close(fd);
}


void doError(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg){
    char header[MAXLINE], body[MAXLINE];

    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n</p>", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny web server</em>\r\n", body);

    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(header, "%sServer: ygxqqx\r\n", header);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sConnection: close\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));
    logInfo("header  = \n %s\n", header);
    rioWriten(fd, header, strlen(header));
    rioWriten(fd, body, strlen(body));
    logInfo("leave clienterror\n");
    return;
}



void parseUri(char *uri, int uri_length, char *filename, char *querystring){
	char *question_mark = strchr(uri, '?');
    int file_length;
    if (question_mark) {
        file_length = (int)(question_mark - uri);
    } else {
        file_length = uri_length;
    }
    
    strcpy(filename, ROOT);

    // uri_length can not be too long
    if (uri_length > (SHORTLINE >> 1)) {
        logErr("uri too long: %.*s", uri_length, uri);
        return;
    }

    strncat(filename, uri, file_length);

    char *last_comp = strrchr(filename, '/');
    char *last_dot = strrchr(last_comp, '.');
    if (last_dot == NULL && filename[strlen(filename)-1] != '/') {
        strcat(filename, "/");
    }
    
    if(uri[strlen(uri)-1] == '/' || filename[strlen(filename)-1] == '/') {
        strcat(filename, "index.html");
    }

    logInfo("filename = %s", filename);
    return;
}


void serveStatic(int fd, char *filename, size_t filesize, yg_http_out_t *out){
    char header[MAXLINE];
    char buf[SHORTLINE];
    int n;
    struct tm tm;
    
    const char *file_type;
    const char *dot_pos = strrchr(filename, '.');
    file_type = getFileType(dot_pos);

	sprintf(header, "HTTP/1.1 %d %s\r\n", out->status, get_shortmsg_from_status_code(out->status));
    
	if (out->keep_alive) {
        sprintf(header, "%sConnection: keep-alive\r\n", header);
    }

        if (out->modified) {
        sprintf(header, "%sContent-type: %s\r\n", header, file_type);
        sprintf(header, "%sContent-length: %zu\r\n", header, filesize);
        localtime_r(&(out->mtime), &tm);
        strftime(buf, SHORTLINE,  "%a, %d %b %Y %H:%M:%S GMT", &tm);
        sprintf(header, "%sLast-Modified: %s\r\n", header, buf);
    } else {

    }

    sprintf(header, "%sServer: ygxqqx\r\n", header);
    sprintf(header, "%s\r\n", header);

    n = rioWriten(fd, header, strlen(header));
    check(n == strlen(header), "rioWriten error, errno = %d", errno);
    if (n != strlen(header)) {
        logErr("n != strlen(header)");
        goto out; 
    }

    if (!out->modified) {
        goto out;
    }


    int srcfd = open(filename, O_RDONLY, 0);
    check(srcfd > 2, "open error");
    // can use sendfile
    char *srcaddr = mmap(NULL, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    check(srcaddr > 0, "mmap error");
    close(srcfd);

    n = rioWriten(fd, srcaddr, filesize);
    // check(n == filesize, "rioWriten error");

    munmap(srcaddr, filesize);

out:
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