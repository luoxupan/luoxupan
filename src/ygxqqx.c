#include <stdint.h>
#include <getopt.h>
#include <signal.h>
#include <string.h>
#include "util.h"
#include "http.h"
#include "epoll.h"
#include "threadpool.h"

#define CONF "ygxqqx.conf"
#define PROGRAM_VERSION "0.1"

extern struct epoll_event *events;

static const struct option long_options[]=
{
    {"help",no_argument,NULL,'?'},
    {"version",no_argument,NULL,'V'},
    {"conf",required_argument,NULL,'c'},
    {NULL,0,NULL,0}
};


static void usage() {
   fprintf(stderr,
	"zaver [option]... \n"
	"  -c|--conf <config file>  Specify config file. Default ./zaver.conf.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
}


int main(int argc, char* argv[]){

    int rc;
    int opt = 0;
    int options_index = 0;
    char *conf_file = CONF;

    if (argc == 1) {
        usage();
        return 0;
    }

    while ((opt=getopt_long(argc, argv,"Vc:?h",long_options,&options_index)) != EOF) {
        switch (opt) {
            case  0 : break;
            case 'c':
                conf_file = optarg;
                break;
            case 'V':
                printf(PROGRAM_VERSION"\n");
                return 0;
            case ':':
            case 'h':
            case '?':
                usage();
                return 0;
        }
    }

    deBug("conffile = %s", conf_file);

    if (optind < argc) {
        logErr("non-option ARGV-elements: ");
        while (optind < argc)
            logErr("%s ", argv[optind++]);
        return 0;
    }

    char conf_buf[BUFLEN];
    yg_conf_t cf;
    rc = readConf(conf_file, &cf, conf_buf, BUFLEN);
    check(rc == YG_CONF_OK, "read conf err");

    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE,&sa,NULL)) {
        logErr("install sigal handler for SIGPIPI failed");
        return 0;
    }

	int sockfd;
    struct sockaddr_in clientaddr;
    // initialize clientaddr and inlen to solve "accept Invalid argument" bug
    socklen_t inlen = 1;
    memset(&clientaddr, 0, sizeof(struct sockaddr_in));  
    
    sockfd = startServer(cf.port);
    rc = makeSocketNoBlocking(sockfd);
    check(rc == 0, "makeSocketNoBlocking");

    int epfd = yg_epoll_create(0);
    struct epoll_event event;
    
    yg_http_request_t *request = (yg_http_request_t *)malloc(sizeof(yg_http_request_t));
    yg_init_request_t(request, sockfd, &cf);

    event.data.ptr = (void *)request;
    event.events = EPOLLIN | EPOLLET;
    yg_epoll_add(epfd, sockfd, &event);

	yg_threadpool_t *tp = threadpool_init(cf.thread_num);










 while (1) {
        int n;
        n = yg_epoll_wait(epfd, events, MAXEVENTS, -1);
        
        int i, fd;
        for (i=0; i<n; i++) {
            yg_http_request_t *r = (yg_http_request_t *)events[i].data.ptr;
            fd = r->fd;
            
            if (sockfd == fd) {
                /* we hava one or more incoming connections */

                while(1) {
                    deBug("## ready to accept");
                    int infd = accept(sockfd, (struct sockaddr *)&clientaddr, &inlen);
                    if (infd == -1) {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                            /* we have processed all incoming connections */
                            break;
                        } else {
                            logErr("accept");
                            break;
                        }
                    }

                    rc = makeSocketNoBlocking(infd);
                    check(rc == 0, "makeSocketNoBlocking");
                    deBug("new connection fd %d", infd);
                    
                    yg_http_request_t *request = (yg_http_request_t *)malloc(sizeof(yg_http_request_t));
                    if (request == NULL) {
                        logErr("malloc(sizeof(yg_http_request_t))");
                        break;
                    }

                    yg_init_request_t(request, infd, &cf);
                    event.data.ptr = (void *)request;
                    event.events = EPOLLIN | EPOLLET;

                    yg_epoll_add(epfd, infd, &event);
                }   // end of while of accept

                deBug("## end accept");
            } else {
                if ((events[i].events & EPOLLERR) ||
                    (events[i].events & EPOLLHUP) ||
                    (!(events[i].events & EPOLLIN))) {
                    logErr("epoll error fd: %d", r->fd);
                    close(fd);
                    continue;
                }
                /*
                do_request(infd);
                close(infd);
                */
                logInfo("new task from fd %d", fd);
                rc = threadpool_add(tp, doRequest, events[i].data.ptr);
                check(rc == 0, "threadpool_add");
            }
        }   //end of for
    }   // end of while(1)
    
    if (threadpool_destroy(tp, 1) < 0) {
        logErr("destroy threadpool failed");
    }

    return 0;
}

