
/*
 * Copyright (C) Zhu Jiashun
 * Copyright (C) Zaver
 */

#include <math.h>
#include "http.h"
#include "http_request.h"

static int yg_http_process_ignore(yg_http_request_t *r, yg_http_out_t *out, char *data, int len);
static int yg_http_process_connection(yg_http_request_t *r, yg_http_out_t *out, char *data, int len);
static int yg_http_process_if_modified_since(yg_http_request_t *r, yg_http_out_t *out, char *data, int len);

yg_http_header_handle_t yg_http_headers_in[] = {
    {"Host", yg_http_process_ignore},
    {"Connection", yg_http_process_connection},
    {"If-Modified-Since", yg_http_process_if_modified_since},
    {"", yg_http_process_ignore}
};

int yg_init_request_t(yg_http_request_t *r, int fd, yg_conf_t *cf) {
    r->fd = fd;
    r->pos = r->last = r->buf;
    r->state = 0;
    r->root = cf->root;
    INIT_LIST_HEAD(&(r->list));

    return YG_OK;
}

int yg_free_request_t(yg_http_request_t *r) {
    // TODO
    return YG_OK;
}

int yg_init_out_t(yg_http_out_t *o, int fd) {
    o->fd = fd;
    o->keep_alive = 0;
    o->modified = 1;
    o->status = 0;

    return YG_OK;
}

int yg_free_out_t(yg_http_out_t *o) {
    // TODO
    return YG_OK;
}

void yg_http_handle_header(yg_http_request_t *r, yg_http_out_t *o) {
    list_head *pos;
    yg_http_header_t *hd;
    yg_http_header_handle_t *header_in;

    list_for_each(pos, &(r->list)) {
        hd = list_entry(pos, yg_http_header_t, list);
        /* handle */

        for (header_in = yg_http_headers_in; 
            strlen(header_in->name) > 0;
            header_in++) {
            if (strncmp(hd->key_start, header_in->name, hd->key_end - hd->key_start) == 0) {
            
                deBug("key = %.*s, value = %.*s", hd->key_end-hd->key_start, hd->key_start, hd->value_end-hd->value_start, hd->value_start);
                int len = hd->value_end-hd->value_start;
                (*(header_in->handler))(r, o, hd->value_start, len);
                break;
            }    
        }

        /* delete it from the original list */
        list_del(pos);
        free(hd);
    }
}

static int yg_http_process_ignore(yg_http_request_t *r, yg_http_out_t *out, char *data, int len) {
    
    return YG_OK;
}

static int yg_http_process_connection(yg_http_request_t *r, yg_http_out_t *out, char *data, int len) {
    if (strncasecmp("keep-alive", data, len) == 0) {
        out->keep_alive = 1;
    }

    return YG_OK;
}

static int yg_http_process_if_modified_since(yg_http_request_t *r, yg_http_out_t *out, char *data, int len) {
    struct tm tm;
    strptime(data, "%a, %d %b %Y %H:%M:%S GMT", &tm);
    time_t client_time = mktime(&tm);

    double time_diff = difftime(out->mtime, client_time);
    if (fabs(time_diff) < 1e6) {
        deBug("not modified!!");
        /* Not modified */
        out->modified = 0;
        out->status = YG_HTTP_NOT_MODIFIED;
    }
    
    return YG_OK;
}

const char *get_shortmsg_from_status_code(int status_code) {
    /*  for code to msg mapping, please check: 
    * http://users.polytech.unice.fr/~buffa/cours/internet/POLYS/servlets/Servlet-Tutorial-Response-Status-Line.html
    */
    if (status_code = YG_HTTP_OK) {
        return "OK";
    }

    if (status_code = YG_HTTP_NOT_MODIFIED) {
        return "Not Modified";
    }

    if (status_code = YG_HTTP_NOT_FOUND) {
        return "Not Found";
    }
    

    return "Unknown";
}
