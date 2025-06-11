#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>

#define MYPORT "3490"   
#define BACKLOG 10      // pending connections in queue

int main(void)
{
    struct sockaddr_storage their_addr;
    socklen_t addr_size;
    struct addrinfo hints, *res;
    int sockfd, new_fd;


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; 

    int status;
    if (( status = getaddrinfo(NULL, MYPORT, &hints, &res)) != 0 ) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }

    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    int iDontNeedThis = bind(sockfd, res->ai_addr, res->ai_addrlen);
    listen(sockfd, BACKLOG);


    addr_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);

    char buf[10];
    int bytes = recv(new_fd, buf, 10, 0 );
    if(bytes == -1){
        fprintf(stderr, "bytes");
    } else {
        printf("Got %d bytes: %.*s\n", bytes, bytes, buf);
    }

    return 0;
}