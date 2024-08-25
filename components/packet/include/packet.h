#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

#define MAX_PACKET_SIZE 88
#define MAX_SINGLE_READING_SIZE 8

typedef struct {
    uint8_t length;
    char payload[MAX_PACKET_SIZE];
} packet_t;


#endif