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
    static const int FILENAME_LEN = 200;        // max length for file name
    static const int READ_BUFFER_SIZE = 2048;   
    static const int WRITE_BUFFER_SIZE = 1024;  
    // HTTP methods, only supports GET for now
    enum METHOD {GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};
    
    /*
        The state of the main state machine when resolving the client request
        CHECK_STATE_REQUESTLINE: The request line is currently being analyzed
        CHECK_STATE_HEADER: The header line is currently being analyzed
        CHECK_STATE_CONTENT: The content is currently being analyzed
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT };
    /*
        Possible result of server processing HTTP request
        NO_REQUEST          :   The request is incomplete and needs to continue reading customer data
        GET_REQUEST         :   Indicates that a completed customer request has been obtained
        BAD_REQUEST         :   Indicates a syntax error in the customer request
        NO_RESOURCE         :   Indicates that the server has no resources
        FORBIDDEN_REQUEST   :   Indicates that the customer does not have sufficient access rights to the resource
        FILE_REQUEST        :   File request. Obtaining the file succeeded
        INTERNAL_ERROR      :   Indicates an internal server error
        CLOSED_CONNECTION   :   Indicates that the client has closed the connection
    */
    enum HTTP_CODE { NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION };
    
    // The three possible states of reading from the state machine, namely the row state, are: 1. Read a complete row 2. Row error 3. Row data is incomplete
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN };
    http_conn(){}
    ~http_conn(){}
    void init(int sockfd,const sockaddr_in& addr);
    void close_conn();
    void process();
    bool read();
    bool write();
    static int m_epollfd;
    static int m_user_count;

private:
    void init();    
    HTTP_CODE process_read();    // Process HTTP request
    bool process_write( HTTP_CODE ret );    // Fill HTTP reply

    // The following set of functions is called by process_read to process the HTTP request
    HTTP_CODE parse_request_line( char* text );
    HTTP_CODE parse_headers( char* text );
    HTTP_CODE parse_content( char* text );
    HTTP_CODE do_request();
    char* get_line() { return m_read_buf + m_start_line; }
    LINE_STATUS parse_line();

    // This set of functions is called by process_write to fill in the HTTP response.
    void unmap();
    bool add_response( const char* format, ... );
    bool add_content( const char* content );
    bool add_content_type();
    bool add_status_line( int status, const char* title );
    bool add_headers( int content_length );
    bool add_content_length( int content_length );
    bool add_linger();
    bool add_blank_line();
    int m_sockfd;           
    sockaddr_in m_address;  // client's socket address
    
    char m_read_buf[ READ_BUFFER_SIZE ];   
    int m_read_idx;                         // Identifies the next location of the last byte of client data that has been read in the read buffer
    int m_checked_idx;                      // The position in the buffer where the character currently being analyzed is read
    int m_start_line;                       // The starting position of the row currently being parsed

    CHECK_STATE m_check_state;              
    METHOD m_method;                        

    char m_real_file[ FILENAME_LEN ];       // The full path to the target file requested by the client, whose contents are equal to doc_root + m_url, where doc_root is the site root directory
    char* m_url;                            // The name of the target file requested by the clinet
    char* m_version;                        // HTTP protocol version number, we only support HTTP1.1 
    char* m_host;                           // host name
    int m_content_length;                   // The total length of the HTTP request message
    bool m_linger;                          // Whether the HTTP request requires maintaining a connection

    char m_write_buf[ WRITE_BUFFER_SIZE ];  
    int m_write_idx;                        // The number of bytes to be sent in the write buffer
    char* m_file_address;                   // The target file requested by the client is mmapped to the start location in memory
    struct stat m_file_stat;                // The status of the destination file. Through it, we can determine whether the file exists, whether it is a directory, whether it is readable, and obtain information such as the size of the file
    struct iovec m_iv[2];                   // We will use writev to perform the write operation, so define the following two members, 
    int m_iv_count;                         // where m_iv_count represents the number of memory blocks to be written.

    int bytes_to_send;              
    int bytes_have_send;            
}; 


#endif
