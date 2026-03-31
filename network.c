#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <errno.h>

#include "network.h"
#include "utils.h"

#define VERBOSE 1

int send_file(peerinfo_t peer, char *fpath) {
    FILE *f = fopen(fpath, "rb");
    if (f == NULL) {
        fprintf(stderr, "ERROR: send_file: Specified file not found!\n");
        return -1;
    }
    
    int bytes_read;

    // Send START packet
    if (send_init_packet(peer, fpath, f) == -1) {
        fprintf(stderr, "ERROR: send_file: File transfer failed!\n");
        return -1;
    }

    packet_t p;
    char buffer[MAX_PACKET_BUFFER_SIZE];
    char chunk[MAX_DATA_SIZE];
    
    // Read entire file one chunk at a time
    while (1) {
        bytes_read = fread(chunk, sizeof(char), MAX_DATA_SIZE, f);
    
        if (bytes_read == 0) {
            // End of file
            if (feof(f) != 0) {
                break;
            } else {
                fprintf(stderr, "ERROR: send_file: Reading file failed!\n");
                return -1;
                
            }
        }

        // Send DATA packet
        p.type = DATA;
        p.data_len = bytes_read;
        memcpy(p.data, chunk, bytes_read);
    
        if (send_packet(peer, &p, buffer) == -1) {
            fprintf(stderr, "ERROR: send_file: File transfer failed!\n");
            return -1;
        }
    }

    // Send END packet
    p.type = END;
    p.data_len = 0;
    p.data[0] = 0x00;

    if (send_packet(peer, &p, buffer) == -1) {
        fprintf(stderr, "ERROR: send_file: File transfer failed!\n");
        return -1;
    }

    fclose(f);
    return 0;
}

int send_packet(peerinfo_t peer, packet_t *p, char *buffer) {
    serialize_packet(p, buffer);
    while (1) {
        int bytes_sent = send(peer.sock, buffer, HEADER_SIZE + p->data_len, 0);
        if (bytes_sent == -1) {
            return -1;
        }

        if (VERBOSE) {
            printf("INFO: packet type %d sent!\n", p->type);
        }
    
        int status = recv_ack(peer);
        if (status == 1) { // ACK not received
            continue; // resend packet again
        } else if (status == -1) {
            fprintf(stderr, "ERROR: send_packet: File transfer failed!\n");
            return -1;
        }
        
        break;
    }

    return 0;
}

int recv_ack(peerinfo_t peer) {
    char buffer[MAX_PACKET_BUFFER_SIZE];

    packet_t p;

    struct sockaddr_in new_addr;
    socklen_t new_addr_len = sizeof(new_addr);

    
    if (recvfrom(peer.sock, buffer, MAX_PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&new_addr, &new_addr_len) == -1) {
        // ACK timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            if (VERBOSE) {
                printf("\nINFO: ACK not received! Resending packet...\n");
            }
            return 1;
        }
        return -1;
    }

    deserialize_packet(buffer, &p);

    // Check peer address
    if (peer.addr_len != new_addr_len ||
        ipcmp(&peer.addr, &new_addr) != 0
    ) {
        return 1;
    }

    if (p.type != ACK) {
        return 1;
    }

    if (VERBOSE) {
        printf("\tINFO: ACK received!\n");  
    }
    return 0;
}

int send_init_packet(peerinfo_t peer, char *fpath, FILE *stream) {
    char *fname = get_filename(fpath);
    uint32_t fsize = get_file_size(stream);
    int fsize_len = snprintf(NULL, 0, "%u", fsize);
    
    packet_t p;
    p.type = START;
    p.data_len = fsize_len + 1 + strlen(fname) + 1;
    snprintf(p.data, p.data_len, "%s\n%u", fname, fsize);

    char buffer[MAX_PACKET_BUFFER_SIZE];
    
    if (send_packet(peer, &p, buffer) == -1) {
        fprintf(stderr, "ERROR: send_init_packet: File transfer failed!\n");
        return -1;
    }

    return 0;
}

int recv_file(int sock) {
    peerinfo_t peer = {0};
    peer.sock = sock;
    peer.addr_len = sizeof(peer.addr);

    // Receive START packet
    packet_t p;

    if (recv_init_packet(&peer, &p) == -1) {
        fprintf(stderr, "ERROR: recv_file: File reception failed!\n");
        return -1;
    }

    char fname[MAX_FPATH_SIZE + 1];
    uint32_t fsize;

    extract_start_data(&p, fname, &fsize);

    FILE *f = fopen(fname, "wb");

    int bytes_written;
    // Receive DATA packets
    while (1) {
        if (recv_packet(peer, &p) == -1) {
            return -1;
        }

        if (p.type == END) {
            break;
        } else if (p.type == DATA) {
            bytes_written = fwrite(p.data, sizeof(char), p.data_len, f);
            if (bytes_written == 0) {
                fprintf(stderr, "ERROR: recv_file: Writing file failed!\n");
                fclose(f);
            }
        }

        // else { invalid packet }
    }

    fclose(f);
    return 0;
}

int recv_packet(peerinfo_t peer, packet_t *p) {
    char buffer[MAX_PACKET_BUFFER_SIZE];

    struct sockaddr_in new_addr;
    socklen_t new_addr_len = sizeof(new_addr);
    
    if (recvfrom(peer.sock, buffer, MAX_PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&new_addr, &new_addr_len) == -1) {
        // ACK timeout
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 1;
        }
        return -1;
    }

    // Check peer address
    if (peer.addr_len != new_addr_len ||
        ipcmp(&peer.addr, &new_addr) != 0
    ) {
        return 1;
    }

    deserialize_packet(buffer, p);

    if (VERBOSE) {
        if (p->type == END) {
            printf("INFO: END packet!\n");
        } else if (p->type == DATA) {
            printf("INFO: DATA packet: %u\n", p->data_len);
        }
    }

    if (send_ack(peer) == -1) {
        perror("send_ack");
        fprintf(stderr, "ERROR: recv_packet: Failed to send ACK!\n");
        return -1;
    }
    
    return 0;
}

int send_ack(peerinfo_t peer) {
    packet_t ack;
    ack.type = ACK;
    ack.data_len = 0;
    ack.data[0] = 0x00;

    char buffer[MAX_PACKET_BUFFER_SIZE];

    serialize_packet(&ack, buffer);

    if (sendto(peer.sock, buffer, HEADER_SIZE + ack.data_len, 0, (struct sockaddr *)&peer.addr, peer.addr_len) == -1) {
        return -1;
    }

    if (VERBOSE) {
        printf("INFO: ACK sent!\n");
    }

    return 0;
}

int recv_init_packet(peerinfo_t *peer, packet_t *p) {
    char buffer[MAX_PACKET_BUFFER_SIZE];
        
    int msg_len = recvfrom(peer->sock, buffer, MAX_PACKET_BUFFER_SIZE, 0, (struct sockaddr *)&peer->addr, &peer->addr_len);

    if (msg_len == -1) {
        perror("recvfrom");
        fprintf(stderr, "ERROR: recv_init_packet: START packet not received!\n");
        return -1;
    }

    if (VERBOSE) {
        printf("INFO: init packet received!\n");
    }

    if (send_ack(*peer) == -1) {
        perror("recv_init_packet");
        fprintf(stderr, "ERROR: recv_init_packet: Failed to send ACK!\n");
        return -1;
    }
    
    deserialize_packet(buffer, p);
    return 0;
}

void serialize_packet(packet_t *p, char *buffer) {
    uint32_t type_net = htonl((uint32_t)p->type);
    uint32_t len_net = htonl(p->data_len);

    memcpy(buffer, &type_net, sizeof(type_net));
    memcpy(buffer + 4, &len_net, sizeof(len_net));
    memcpy(buffer + 8, p->data, p->data_len);
}

void deserialize_packet(char *buffer, packet_t *p) {
    uint32_t type_net, len_net;
    memcpy(&type_net, buffer, 4);
    memcpy(&len_net, buffer + 4, 4);
    
    p->type = (enum PacketType)ntohl(type_net);
    p->data_len = ntohl(len_net);
    memcpy(p->data, buffer + 8, p->data_len);
}

int ipcmp(struct sockaddr_in *ip1, struct sockaddr_in *ip2) {
    if (ip1->sin_family == ip2->sin_family &&
        ip1->sin_port == ip2->sin_port &&
        ip1->sin_addr.s_addr == ip2->sin_addr.s_addr
    ) {
        return 0;
    }

    if (VERBOSE) {
        printf("INFO: Packet received from unknown address!\n");
    }

    return 1;
}

void extract_start_data(packet_t *p, char *fname, uint32_t *fsize) {
    sscanf(p->data, "%" XSTR(MAX_FPATH_SIZE) "s %u", fname, fsize);
}
