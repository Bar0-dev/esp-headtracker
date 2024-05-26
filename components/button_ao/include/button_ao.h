#ifndef BUTTON_AO_H
#define BUTTON_AO_H

#include "esp_log.h"
#include "esp_ao.h"
#include "events_broker.h"

#define HOLD_TIME 700
#define DOUBLE_PRESS_TIME 200

typedef struct
{
    Active super;

    TimeEvent holdTimer;
    TimeEvent doublePressTimer;
} Button;

enum ButtonEventSignals
{
    BUTTON_HOLD_TIMEOUT_SIG = LAST_EVENT_FLAG,
    BUTTON_DOUBLE_PRESS_TIMEOUT_SIG,
};

void Button_ctor(Button * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;

#endif