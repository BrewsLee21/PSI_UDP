#include <stdint.h>
#include <sys/socket.h>

#ifndef UTILS_H
#define UTILS_H

#define MAX_DATA_SIZE 1024

enum PacketType {
    ACK, // Acknowledgement
    START, // Start of data
    DATA, // Data
    END, // End of data
    
};

typedef struct packet {
	enum PacketType type; // packet type
	uint32_t data_len;
	char data[MAX_DATA_SIZE];
} packet_t;

#endif
