#ifndef COMS_AO_H
#define COMS_AO_H

#include "esp_ao.h"
#include "prov_mgr.h"
#include "events_broker.h"

enum CommsPrivateSignals
{
    WIFI_CONNECTED_SIG = LAST_EVENT_FLAG,
};

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