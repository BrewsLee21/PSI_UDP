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

    if (getaddrinfo("localhost", PORT, &hints, &res) != 0) {
        fprintf(stderr, "ERROR: getaddrinfo() call failed!\n");
        return 1;
    }
    
    int my_socket;
    
    // Create socket
    struct addrinfo *p;
    for (p = res; p != NULL; p = p->ai_next) {

        if ((my_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }

        break;
    }
    
    freeaddrinfo(res);

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

    if (connect(my_socket, p->ai_addr, p->ai_addrlen)) {
        fprintf(stderr, "ERROR: Failed to connect to receiver!\n");
        close(my_socket);
        return 1;
    }

    peerinfo_t peer;

    printf("sa_family: %u\n", p->ai_addr->sa_family);
    peer.sock = my_socket;
    peer.addr = (struct sockaddr_in *)p->ai_addr;
    peer.addr_len = p->ai_addrlen;

    // TODO: fpath instead and extract fname
    char fname[MAX_FNAME_SIZE + 1];

    printf("Enter file path (%d bytes): ", MAX_FNAME_SIZE);
    scanf("%" XSTR(MAX_FNAME_SIZE) "s", fname);
    
    printf("Sending file: %s\n", fname);

    FILE *f = fopen(fname, "rb");

    send_init_packet(peer, fname, f);

    close(my_socket);   
    fclose(f); 
    return 0;
}
