#ifndef BUTTON_AO_H
#define BUTTON_AO_H

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_ao.h"
#include "events_broker.h"

#define BUTTON_PIN 17
#define POLL_TIME 50
#define DEBOUNCE_TIME 15
#define HOLD_TIME 700
#define DOUBLE_PRESS_TIME 200

typedef struct
{
    Active super;
    bool buttonPrevState;

    TimeEvent pollTimer;
    TimeEvent debounceTimer;
    TimeEvent holdTimer;
    TimeEvent doublePressTimer;
} Button;

enum ButtonEventSignals
{
    BUTTON_POLLING_TIMEOUT_SIG = LAST_EVENT_FLAG,
    BUTTON_DEBOUNCE_TIMEOUT_SIG,
    BUTTON_HOLD_TIMEOUT_SIG, 
    BUTTON_DOUBLE_PRESS_TIMEOUT_SIG,
    BUTTON_STATE_CHANGED_SIG,
};

void Button_ctor(Button * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;

#endif