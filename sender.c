#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include <arpa/inet.h>

#include <unistd.h>

#include "network.h"

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

    printf("Sending to: %s\n", receiver_addr);
    
    int my_socket;
    
    // Create socket
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {

        if ((my_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        break;
    }

    struct timeval tv;
    tv.tv_sec = ACK_TIMEOUT;
    tv.tv_usec = 0;

    if (p == NULL) {
        fprintf(stderr, "ERROR: Failed to create socket!\n");
        return 1;
    }

    if (setsockopt(my_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) == -1) {
        fprintf(stderr, "ERROR: Failed to set socket options!\n");
        return 1;
    }

    /*
    if (connect(my_socket, p->ai_addr, p->ai_addrlen)) {
        fprintf(stderr, "ERROR: Failed to connect to receiver!\n");
        close(my_socket);
        return 1;
    }
    */

    peerinfo_t peer;
    peer.sock = my_socket;
    memcpy(&peer.addr, p->ai_addr, sizeof(struct sockaddr_in));
    peer.addr_len = p->ai_addrlen;

    char fpath[MAX_FPATH_SIZE + 1];

    printf("Enter file path (%d bytes): ", MAX_FPATH_SIZE);
    scanf("%" XSTR(MAX_FPATH_SIZE) "s", fpath);
    
    printf("Sending file: %s\n", fpath);

    if (send_file(peer, fpath) == -1) {
        close(my_socket);
        return 1;
    }

    printf("File transmitted successfully!\n");

    freeaddrinfo(res);
    close(my_socket);
    return 0;
}
