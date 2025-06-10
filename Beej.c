/*
    This is a test file. Running examples and testing out functions.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>




int main (void) {
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;

    memset(&hints, 0, sizeof hints); // make sure struct is empty
    hints.ai_family = AF_UNSPEC; // v4 or v6
    hints.ai_socktype = SOCK_STREAM; // TCP 
    hints.ai_flags = AI_PASSIVE; // fill in ip for me

    //int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
    if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0 ) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }
    // servinfo now points to a linked list of 1 or more struct addrinfos
    

    freeaddrinfo(servinfo); // free the linked-list

    return 0;
}




int printRouterInfo() {
    // SLOP //
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET_ADDRSTRLEN];

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return 1;

    }


    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);

            // Filter out loopback and only print actual IPs
            if (strcmp(ip, "127.0.0.1") != 0) {
                printf("Interface %s: %s\n", ifa->ifa_name, ip);
            }
        }
    }

    freeifaddrs(ifaddr);
    return 0;
}