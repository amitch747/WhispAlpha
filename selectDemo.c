/*
selectDemo.c -- testing select()
*/

#include <bits/types/struct_timeval.h>
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define STDIN 0 // file descriptor for standard input

int main(void) 
{

    struct timeval tv; 
    fd_set readfds;
    char c;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);

    select(STDIN+1, &readfds, NULL, NULL, &tv);

    if (FD_ISSET(STDIN, &readfds)) {
        if (read(STDIN, &c, 1)>0 ) {
            
            printf("[%c] pressed\n",c);
        }
    }
    
    else printf("Timeout\n");


    return 0;
}

