#ifndef LED_AO_H
#define LED_AO_H

#include "esp_ao.h"
#include "events_broker.h"

#define LED_PIN 2

typedef enum
{
    LED_OFF,
    LED_ON,
} LedState_t;

typedef enum
{
    BLINK_FAST = 100,
    BLINK_MEDIUM = 500,
    BLINK_SLOW = 1000,
} BlinkPeriod_t;

typedef struct
{
    Active super;
    bool ledState;
    BlinkPeriod_t blinkPeriod;
    TimeEvent ledTimer;
} Led;

enum LedEventSignals
{
    BLINK_TIMER_EXPIRED_SIG = LAST_EVENT_FLAG,
};

void Led_ctor(Led * const me);

#endif