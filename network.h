#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifndef NETWORK_H
#define NETWORK_H

#define ACK_TIMEOUT 5

#define MAX_DATA_SIZE 1024
#define PORT "8080"

#define HEADER_SIZE (sizeof(enum PacketType) + sizeof(uint32_t)) // enum + uint32_t
#define MAX_PACKET_BUFFER_SIZE (HEADER_SIZE + MAX_DATA_SIZE)

#define MAX_FPATH_SIZE 255

#define STR(x) #x
#define XSTR(x) STR(x)


/**
* ACK
*   Sent by receiver back to sender upon receiving a packet.
*   If sender doesn't get ACK packet after seding data, sender resends
*       the packet again. (and again... until ACK is received)
*
* START
*   Signalizes that the following packets will contain file data.
*   This packet's data should contain file information:
*       1st line: <file_name>
*       2nd line: <file_size_bytes>
*
* DATA
*   Contains raw file data.
*
* END
*   Signalizes end of file transfer.
*   Receiver should be able to store the complete file.
*/

enum PacketType {
    ACK, // Acknowledgement
    START, // Start of data.
    DATA, // Data
    END, // End of data
    
};

typedef struct packet {
	enum PacketType type; // packet type
	uint32_t data_len; // length of data (in bytes)
	char data[MAX_DATA_SIZE];
} packet_t;

typedef struct peerinfo {
    int sock; // socket to peer
    struct sockaddr_in addr; // peer address
    socklen_t addr_len; // peer address length
} peerinfo_t;

/**
* Functions called by sender
*/
int send_file(peerinfo_t peer, char *fpath);
int send_packet(peerinfo_t peer, packet_t *p, char *buffer);
int recv_ack(peerinfo_t peer);
int send_init_packet(peerinfo_t peer, char *fpath, FILE *stream);

/**
* Functions called by receiver
*/
int recv_file(int sock);
int recv_packet(peerinfo_t peer, packet_t *p);
int send_ack(peerinfo_t peer);
int recv_init_packet(peerinfo_t *peer, packet_t *p);

/**
* Serialization for sending data on a socket
*/
void serialize_packet(packet_t *p, char *buffer);
void deserialize_packet(char *buffer, packet_t *p);

void extract_start_data(packet_t *p, char *fname, uint32_t *fsize);

/**
* Compares two IPv4 addresses
*/
int ipcmp(struct sockaddr_in *ip1, struct sockaddr_in *ip2);

#endif
