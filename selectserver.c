/*
selectserver.c -- multiperson select() server
*/

#include <asm-generic/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"

// get sockaddr v4 or v6
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main (void)
{   
    fd_set master;      // master file descriptor list
    fd_set read_fds;    // temp file descriptor list for select()
    int fdmax;

    int listener;       // listening socket descriptor
    int newfd;          // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256];
    int nbytes;

    char remoteIP[INET6_ADDRSTRLEN];

    int yes=1;
    int i, j, rv;

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master);   // clear sets
    FD_ZERO(&read_fds);


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close (listener);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai);


    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add listener to master set
    FD_SET(listener, &master);

    // keep track of biggest file descriptor
    fdmax = listener; // just this so far

    // main loop
    for (;;) {
        read_fds = master; // copy over each time
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on socket %d\n", 
                            inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr),
                            remoteIP,INET6_ADDRSTRLEN),newfd);
                    }
                } else {
                    // i that is ready corresponds to a client socket
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socked %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i);
                        FD_CLR(i, &master); // remove from master
                    } else {
                        // good data
                        for (j = 0; j <= fdmax; j++) {
                            // send to everyone
                            if (FD_ISSET(j, &master)) {
                                // except the listener and ourselves
                                if (j != listener && j != i) {
                                    if (send(j, buf, nbytes, 0) == -1){
                                        perror("send");
                                    }
                                }
                            }
                        }
                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)
    return 0;
}


