#ifndef POLLING_AO_H
#define POLLING_AO_H

#include "driver/gpio.h"
#include "esp_ao.h"
#include "events_broker.h"

#define BUTTON_PIN 17
#define POLL_TIME 50
#define DEBOUNCE_TIME 15

typedef struct
{
    Active super;
    bool buttonPrevState;

    TimeEvent pollTimer;
    TimeEvent debounceTimer;
} Polling;

enum PollingEventSignals
{
    POLLING_TIMER_SIG = USER_SIG,
    BUTTON_DEBOUNCED_SIG,
};

void Polling_ctor(Polling * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;
#endif