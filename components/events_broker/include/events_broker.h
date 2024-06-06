#ifndef EVENTS_BROKER_H
#define EVENTS_BROKER_H

#include "esp_ao.h"

#define MAX_AOS_PER_EVENT 10

typedef enum 
{
    EV_BUTTON_PRESSED = USER_SIG,
    EV_BUTTON_RELEASED,
    EV_BUTTON_DOUBLE_PRESS,
    EV_BUTTON_HOLD,
    LAST_EVENT_FLAG,
} GlobalSignal_t;

typedef struct 
{
    Event super;
    Active *aos[MAX_AOS_PER_EVENT];
    uint8_t subscribers;
} GlobalEvent_t;

typedef struct
{
    Active super;
    GlobalEvent_t globalEvents[LAST_EVENT_FLAG-USER_SIG];
} Broker;

void Broker_ctor(Broker * const me);

void Broker_subscribe(Broker * const me, Event const * const e, Active * const ao);

#endif