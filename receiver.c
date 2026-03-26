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

    if (getaddrinfo(NULL, PORT, &hints, &res) != 0) {
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

        if (bind(my_socket, p->ai_addr, p->ai_addrlen) == -1) {
            close(my_socket);
            continue;
        }

        break;
    }
    
    freeaddrinfo(res);

    if (p == NULL) {
        fprintf(stderr, "ERROR: Failed to bind to an address!\n");
        return 1;
    }

    struct sockaddr_in sender_addr;
    
    printf("Waiting...\n");

    char data[128];
    int msg_len;
    socklen_t addr_len = sizeof(sender_addr);
    msg_len = recvfrom(my_socket, data, sizeof(data), 0, (struct sockaddr *)&sender_addr, &addr_len);

    if (msg_len == -1) {
        fprintf(stderr, "ERROR: Failed to receive data!\n");
        close(my_socket);
        return 1;
    }

    printf("Data received: %s\n", data);

    close(my_socket);
    return 0;
}
