#ifndef COMS_AO_H
#define COMS_AO_H

#include "esp_ao.h"
#include "prov_mgr.h"
#include "events_broker.h"

typedef struct
{
    Active super;
} Coms;

void Coms_ctor(Coms * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;

#endif