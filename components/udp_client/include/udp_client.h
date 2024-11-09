#ifndef UDP_H
#define UDP_H

#include "core.h"

void udp_client_init(void);
void udp_client_send(char *msg, uint64_t size);

#endif
