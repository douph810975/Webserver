#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include "http_conn.h"
#include "threadpool.h"
#include <sys/uio.h>
#define MAX_FD 65536   // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量

extern void addfd();
extern void removefd();

int main(int argc, char* argv[]){
    if(argc<=1){
        printf("Enter port name");
        return 1;
    }
    int port = atoi(argv[1]);
    //addsig(SIGPIPE, SIG_IGN);
    threadpool<http_conn>* pool = NULL;
    try
    {
        pool = new threadpool<http_conn>;
    }
    catch( ... )
    {
        return 1;
    }
    http_conn* users = new http_conn[MAX_FD];
    int listenfd = socket(PF_INET,SOCK_STREAM,0);
    int ret = 0;
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

}
