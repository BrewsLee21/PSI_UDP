#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>

#include <unistd.h>

#include "utils.h"


int main() {
    struct addrinfo *res, hints;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_DGRAM; // UDP
    hints.ai_flags = AI_PASSIVE; // Any available local address

    char receiver_addr[INET6_ADDRSTRLEN];

    printf("Enter receiver IP address: ");
    scanf("%s", receiver_addr);
    getchar(); // Consume '\n'

    if (getaddrinfo(receiver_addr, PORT, &hints, &res) != 0) {
        fprintf(stderr, "ERROR: getaddrinfo() call failed!\n");
        return 1;
    }

    //struct sockaddr_in *ipv4;
    //struct sockaddr_in6 *ipv6;
    int my_socket;

    //char ipstr[INET6_ADDRSTRLEN];

    
    // Create socket and bind to the first available address
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {

        // Prints addresses in res
        /*
        if (p->ai_family == AF_INET) {
            ipv4 = (struct sockaddr_in *)p->ai_addr;
            inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
            printf("ipv4: %s\n", ipstr);
        } 
        else if (p->ai_family == AF_INET6) {
            ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof(ipstr));
            printf("ipv6: %s\n", ipstr);
        }
        */

        if ((my_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        break;
    }
    
    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "ERROR: Failed to bind to an address!\n");
        return 1;
    }

    if (connect(my_socket, p->ai_addr, p->ai_addrlen)) {
        fprintf(stderr, "ERROR: Failed to connect to receiver!\n");
        close(my_socket);
        return 1;
    }

    char fname[128];

    printf("Enter file name: ");
    fgets(fname, sizeof(fname), stdin);
    printf("Sending file: %s\n", fname);

    int bytes_sent;
    bytes_sent = send(my_socket, fname, strlen(fname), 0);
    if (bytes_sent == -1) {
        fprintf(stderr, "ERROR: Failed to send data!\n");
        close(my_socket);
        return 1;
    }

    printf("bytes_sent: %d\n", bytes_sent);


    close(my_socket);    
    return 0;
}
