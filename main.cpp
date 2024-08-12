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

extern void addfd(int epollfd, int fd, bool one_shot);
extern void removefd();
void portReuse(int sockfd){
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
}
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
    portReuse(listenfd);
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    ret = listen(listenfd,5);
    // Create epoll instance 
    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    addfd(epollfd, listenfd, false);
    http_conn::m_epollfd = epollfd;

    while(true){
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if((number < 0) && (errno!= EINTR)){
            printf("epoll failure\n");
            break;
        }
        for(int i=0;i<number;i++){
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd){
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                if(connfd < 0){
                    printf("errno is: %d\n", errno);
                    continue;
                }
                if(http_conn::m_user_count >= MAX_FD){
                    close(connfd);
                    continue;
                }
                users[connfd].init(connfd, client_address);
            }else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                users[sockfd].close_conn();
            }else if(events[i].events & EPOLLIN){
                if(users[sockfd].read()){
                    pool->append(users+sockfd);
                }else {
                    users[sockfd].close_conn();
                }
            }else if( events[i].events & EPOLLOUT ) {

                if( !users[sockfd].write() ) {
                    users[sockfd].close_conn();
                }
            }
        }
    }
    close(epollfd);
    close(listenfd);
    delete [] users;
    delete pool;
    return 0;
}


