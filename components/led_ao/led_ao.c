#include "led_ao.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "esp_log.h"

State Led_init(Led * const me, Event const * const e);
State Led_top(Led * const me, Event const * const e);
State Led_idle(Led * const me, Event const * const e);
State Led_blink(Led * const me, Event const * const e);
State Led_blink_fast(Led * const me, Event const * const e);
State Led_blink_medium(Led * const me, Event const * const e);
State Led_blink_slow(Led * const me, Event const * const e);
State Led_solid(Led * const me, Event const * const e);

static const gpio_config_t gpioConfig = 
{
    .pin_bit_mask = (1ULL<<LED_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
};

static void Led_off(void)
{
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));
}

static void Led_on(void)
{
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 1));
}

static void Led_toggle(bool currentState)
{
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, !currentState));
}

State Led_init(Led * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Led_idle);
}

State Led_top(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EV_IMU_IDLE:
        status = transition(&me->super.super, (StateHandler)&Led_idle);
        break;
    
    case EV_IMU_READING:
        status = transition(&me->super.super, (StateHandler)&Led_solid);
        break;
    
    case EV_IMU_CALIBRATION_READY:
        status = transition(&me->super.super, (StateHandler)&Led_blink_slow);
        break;
    
    case EV_IMU_CALIBRATION_IN_PROGRESS:
        status = transition(&me->super.super, (StateHandler)&Led_blink_fast);
        break;
    
    case EV_IMU_CALIBRATION_DONE:
        status = transition(&me->super.super, (StateHandler)&Led_blink_medium);
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}

State Led_idle(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->ledState = LED_OFF;
        Led_off();
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_top);
        break;
    }
    return status;
}

State Led_blink(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->ledTimer);
        status = HANDLED_STATUS;
        break;
    
    case BLINK_TIMER_EXPIRED_SIG:
        Led_toggle(me->ledState);
        me->ledState = !me->ledState;
        status = HANDLED_STATUS;
        break;
        
    case EXIT_SIG:
        TimeEvent_disarm(&me->ledTimer);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_top);
        break;
    }
    return status;
}

State Led_blink_fast(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->blinkPeriod = BLINK_FAST;
        TimeEvent_change_period(&me->ledTimer, (TickType_t)((uint16_t)me->blinkPeriod/portTICK_PERIOD_MS));
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_blink);
        break;
    }
    return status;
}

State Led_blink_medium(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->blinkPeriod = BLINK_MEDIUM;
        TimeEvent_change_period(&me->ledTimer, (TickType_t)((uint16_t)me->blinkPeriod/portTICK_PERIOD_MS));
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_blink);
        break;
    }
    return status;
}

State Led_blink_slow(Led * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->blinkPeriod = BLINK_SLOW;
        TimeEvent_change_period(&me->ledTimer, (TickType_t)((uint16_t)me->blinkPeriod/portTICK_PERIOD_MS));
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_blink);
        break;
    }
    return status;
}

State Led_solid(Led * const me, Event const * const e){
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        Led_on();
        me->ledState = LED_ON;
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Led_top);
        break;
    }
    return status;
}

void Led_ctor(Led * const me)
{
    me->blinkPeriod = BLINK_SLOW;
    Active_ctor(&me->super, (StateHandler)&Led_init);
    TimeEvent_ctor(&me->ledTimer, "LED timer", (TickType_t)(me->blinkPeriod/portTICK_PERIOD_MS), pdTRUE, BLINK_TIMER_EXPIRED_SIG, &me->super);
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
}