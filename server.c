/*
    server.c -- stream socket server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10


void sigchld_handler(int s) // signal handlers apparently need the signal number
{
    (void)s; //quiet unused variable warning;

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while(waitpid(-1, NULL, WNOHANG) > 0);

    errno = saved_errno;
}

// get sockaddr, IPv4 or v6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
    
}

int main(void)
{
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p; 
    struct sockaddr_storage their_addr; // client's address info

    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = AI_PASSIVE; // my ip

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // at this point our own adress info is known. need to make socket and bind to a port, then listen
    // loop through results and bind to first one
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ) {
            perror("sever: socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            // need this to avoid TIME_WAIT. we can thus reuse the PORT right away after restart
            perror("setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // no need for this anymore, we have our socket ready

    
    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap dead guys
    sigemptyset(&sa.sa_mask); // dont block other signals
    sa.sa_flags = SA_RESTART; // if system call is interrupted by sa, restart it
    // Child terminated or stopped. "Install" signal handler just for SIGCHILD
    if (sigaction(SIGCHLD, &sa, NULL) == -1) { 
        perror("sigaction");
        exit(1);
    }

    printf("server: waiting for connections...\n");

 

    while(1) {
        printf("DEBUG: Calling accept()...\n");
        fflush(stdout);
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        printf("new_fd: %d", new_fd);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),s, sizeof(s));
        printf("server: connection from %s\n", s);

        if (!fork()) {
            close(sockfd);
            if (send(new_fd, "Hello world!", 13, 0) == -1) {
                perror("send");                
            }
            close(new_fd);
            exit(0);
        }
        close(new_fd);
    }
    return 0;
}