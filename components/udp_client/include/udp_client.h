#ifndef UDP_H
#define UDP_H

#include "core.h"

void udp_client_init(void);
void udp_client_send(Packet_t * packet);

#endif