
/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */

#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <time.h>
#include "http.h"

#define YG_AGAIN    EAGAIN
#define YG_OK       0

#define YG_HTTP_PARSE_INVALID_METHOD        10
#define YG_HTTP_PARSE_INVALID_REQUEST       11
#define YG_HTTP_PARSE_INVALID_HEADER        12

#define YG_HTTP_UNKNOWN                     0x0001
#define YG_HTTP_GET                         0x0002
#define YG_HTTP_HEAD                        0x0004
#define YG_HTTP_POST                        0x0008

#define YG_HTTP_OK                          200

#define YG_HTTP_NOT_MODIFIED                304

#define YG_HTTP_NOT_FOUND                   404

#define MAX_BUF 8124

typedef struct yg_http_request_s {
    void *root;
    int fd;
    char buf[MAX_BUF];
    void *pos, *last;
    int state;
    void *request_start;
    void *method_end;   /* not include method_end*/
    int method;
    void *uri_start;
    void *uri_end;      /* not include uri_end*/ 
    void *path_start;
    void *path_end;
    void *query_start;
    void *query_end;
    int http_major;
    int http_minor;
    void *request_end;

    struct list_head list;  /* store http header */
    void *cur_header_key_start;
    void *cur_header_key_end;
    void *cur_header_value_start;
    void *cur_header_value_end;

} yg_http_request_t;

typedef struct {
    int fd;
    int keep_alive;
    time_t mtime;       /* the modified time of the file*/
    int modified;       /* compare If-modified-since field with mtime to decide whether the file is modified since last time*/

    int status;
} yg_http_out_t;

typedef struct yg_http_header_s {
    void *key_start, *key_end;          /* not include end */
    void *value_start, *value_end;
    list_head list;
} yg_http_header_t;

typedef int (*yg_http_header_handler_pt)(yg_http_request_t *r, yg_http_out_t *o, char *data, int len);

typedef struct {
    char *name;
    yg_http_header_handler_pt handler;
} yg_http_header_handle_t;

extern void yg_http_handle_header(yg_http_request_t *r, yg_http_out_t *o);
extern int yg_init_request_t(yg_http_request_t *r, int fd, yg_conf_t *cf);
extern int yg_free_request_t(yg_http_request_t *r);

extern int yg_init_out_t(yg_http_out_t *o, int fd);
extern int yg_free_out_t(yg_http_out_t *o);

extern const char *get_shortmsg_from_status_code(int status_code);

extern yg_http_header_handle_t     yg_http_headers_in[];

#endif

