#include "polling_ao.h"
#include "esp_log.h"

static const gpio_config_t gpioConfig =
    {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
};

State Polling_polling(Polling * const me, Event const * const e)
{
    State status;
    static Event evt = { EV_POLLING_BUTTON_STATE_CHANGED, (void*)0 }; 
    static bool buttonCurrentState;
    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->pollTimer);
        status = HANDLED_STATUS;
        break;

    case POLLING_TIMER_SIG:
        buttonCurrentState = (bool)gpio_get_level(BUTTON_PIN);
        if(buttonCurrentState != me->buttonPrevState){
            TimeEvent_arm(&me->debounceTimer);
            TimeEvent_disarm(&me->pollTimer);
        }
        status = HANDLED_STATUS;
        break;
    
    case BUTTON_DEBOUNCED_SIG:
        buttonCurrentState = (bool)gpio_get_level(BUTTON_PIN);
        evt.payload = &buttonCurrentState;
        if(buttonCurrentState != me->buttonPrevState){
            Active_post(AO_Broker, &evt);
            me->buttonPrevState = buttonCurrentState;
        }
        TimeEvent_arm(&me->pollTimer);
        status = HANDLED_STATUS;
        break;

    default:
        status = IGNORED_STATUS;
        break;
    }
    return status;
}

State Polling_init(Polling * const me, Event const * const e){
    me->buttonPrevState = 1;
    return transition(&me->super.super, (StateHandler)&Polling_polling);
}

void Polling_ctor(Polling *const me)
{
    Active_ctor(&me->super, (StateHandler)&Polling_init);
    TimeEvent_ctor(&me->debounceTimer, "Debouce timer", (TickType_t)(DEBOUNCE_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DEBOUNCED_SIG, &me->super);
    TimeEvent_ctor(&me->pollTimer, "Poll timer", (TickType_t)(POLL_TIME / portTICK_PERIOD_MS), pdTRUE, POLLING_TIMER_SIG , &me->super);
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
}