#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

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
#include <sys/uio.h>

class http_conn
{ 
public:
    http_conn(){}
    ~http_conn(){}
    void init(int sockfd,const sockaddr_in& addr);
    void close_conn();
    void process();
    bool read();
    bool write();
    static int m_epollfd;
    static int m_user_count;
}; 


#endif
