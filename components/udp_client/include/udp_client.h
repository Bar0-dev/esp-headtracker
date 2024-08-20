#ifndef UDP_H
#define UDP_H

#include "packet.h"

void udp_client_init(void);
void udp_client_send(packet_t * packet);

#endif