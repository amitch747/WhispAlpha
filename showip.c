/* 
    showip.c -- show IP addresses for a host given on the command line
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) 
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET6_ADDRSTRLEN];

    

    if (argc != 2) {
        fprintf(stderr, "usage: showip hostname\n");
        return 1;
    }



    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // AF_INET OR AF_INET6 also options
    hints.ai_socktype = SOCK_STREAM;


    if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0 ) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 2;
    }

    printf("IP Addresses for %s\n\n", argv[1]);

    for (p = res; p != NULL; p = p->ai_next) {
        void *addr;
        char *ipver;

        if (p->ai_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPV4";
        } else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }
        // convert IP to string and print it: 
        inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);
        printf(" %s: %s\n", ipver, ipstr);

    }
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    int bindOutput = bind(sockfd, res->ai_addr, res->ai_addrlen);

    
    
    return 0;


}
