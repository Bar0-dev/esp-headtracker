#include "button_ao.h"
#include "esp_log.h"

static const gpio_config_t gpioConfig =
    {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
};

State Button_init(Button *const me, Event const *const e);
State Button_polling(Button *const me, Event const *const e);
State Button_check_for_state(Button *const me, Event const *const e);
State Button_released(Button *const me, Event const *const e);
State Button_pressed(Button *const me, Event const *const e);
State Button_pre_doublepressed(Button *const me, Event const *const e);
State Button_doublepressed(Button *const me, Event const *const e);
State Button_hold(Button *const me, Event const *const e);

State Button_init(Button *const me, Event const *const e){
    return transition(&me->super.super, (StateHandler)&Button_check_for_state);
}

State Button_polling(Button *const me, Event const *const e)
{
    State status;
    static Event evt = { BUTTON_STATE_CHANGED_SIG, (void*)0 }; 
    static bool buttonCurrentState;
    switch (e->sig)
    {
        case ENTRY_SIG:
            TimeEvent_arm(&me->pollTimer);
            status = HANDLED_STATUS;
            break;

        case BUTTON_POLLING_TIMEOUT_SIG:
            buttonCurrentState = (bool)gpio_get_level(BUTTON_PIN);
            if(buttonCurrentState != me->buttonPrevState){
                TimeEvent_arm(&me->debounceTimer);
                TimeEvent_disarm(&me->pollTimer);
            }
            status = HANDLED_STATUS;
            break;
        
        case BUTTON_DEBOUNCE_TIMEOUT_SIG:
            buttonCurrentState = (bool)gpio_get_level(BUTTON_PIN);
            if(buttonCurrentState != me->buttonPrevState){
                Active_post(&me->super, &evt);
                me->buttonPrevState = buttonCurrentState;
            }
            TimeEvent_arm(&me->pollTimer);
            status = HANDLED_STATUS;
            break;

        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;

        default:
            status = super(&me->super.super, (StateHandler)&Hsm_top);
            break;
    }
    return status;
}

State Button_check_for_state(Button *const me, Event const *const e){
    State status;
    static bool buttonCurrentState;
    switch (e->sig)
    {
        case ENTRY_SIG:
            buttonCurrentState = (bool)gpio_get_level(BUTTON_PIN);
            me->buttonPrevState = buttonCurrentState;
            if(buttonCurrentState){
                status = transition(&me->super.super, (StateHandler)&Button_released);
            }else{
                status = transition(&me->super.super, (StateHandler)&Button_pressed);
            }
            break;

        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;

        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
    }
    return status;
}

State Button_released(Button *const me, Event const *const e){
    State status;
    Event evt = { EV_BUTTON_RELEASED, (void *)0};
    switch (e->sig)
    {
        case ENTRY_SIG:
            Active_post(AO_Broker, &evt);
            status = HANDLED_STATUS;
            break;

        case BUTTON_STATE_CHANGED_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_pressed);
            break;
        
        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;

        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
    }
    return status;
}

State Button_pressed(Button *const me, Event const *const e){
    State status;
    Event evt = { EV_BUTTON_PRESSED, (void *)0};
    switch (e->sig)
    {
        case ENTRY_SIG:
            TimeEvent_arm(&me->holdTimer);
            Active_post(AO_Broker, &evt);
            status = HANDLED_STATUS;
            break;

        case BUTTON_STATE_CHANGED_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_pre_doublepressed);
            break;
        
        case BUTTON_HOLD_TIMEOUT_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_hold);
            break;
        
        case EXIT_SIG:
            TimeEvent_disarm(&me->holdTimer);
            status = HANDLED_STATUS;
            break;

        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
    }
    return status;
}
State Button_pre_doublepressed(Button *const me, Event const *const e){
    State status;
        switch (e->sig)
        {
        case ENTRY_SIG:
            TimeEvent_arm(&me->doublePressTimer);
            status = HANDLED_STATUS;
            break;
        
        case BUTTON_STATE_CHANGED_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_doublepressed);
            break;
        
        case BUTTON_DOUBLE_PRESS_TIMEOUT_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_released);
            break;

        case EXIT_SIG:
            TimeEvent_disarm(&me->doublePressTimer);
            status = HANDLED_STATUS;
            break;

        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
        }
    return status;
}
State Button_hold(Button *const me, Event const *const e){
    State status;
    Event evt = { EV_BUTTON_HOLD, (void *)0};
        switch (e->sig)
        {
        case ENTRY_SIG:
            Active_post(AO_Broker, &evt);
            status = HANDLED_STATUS;
            break;
        
        case BUTTON_STATE_CHANGED_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_released);
            break;

        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;
            
        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
        }
    return status;
}

State Button_doublepressed(Button *const me, Event const *const e){
    State status;
    Event evt = { EV_BUTTON_DOUBLE_PRESS, (void *)0};
        switch (e->sig)
        {
        case ENTRY_SIG:
            Active_post(AO_Broker, &evt);
            status = HANDLED_STATUS;
            break;

        case BUTTON_STATE_CHANGED_SIG:
            status = transition(&me->super.super, (StateHandler)&Button_released);
            break;
        
        case EXIT_SIG:
            status = HANDLED_STATUS;
            break;
            
        default:
            status = super(&me->super.super, (StateHandler)&Button_polling);
            break;
        }
    return status;
}

void Button_ctor(Button *const me)
{
    Active_ctor(&me->super, (StateHandler)&Button_init);
    TimeEvent_ctor(&me->pollTimer, "Poll timer", (TickType_t)(POLL_TIME / portTICK_PERIOD_MS), pdTRUE, BUTTON_POLLING_TIMEOUT_SIG , &me->super);
    TimeEvent_ctor(&me->debounceTimer, "Debouce timer", (TickType_t)(DEBOUNCE_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DEBOUNCE_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->holdTimer, "Hold timer", (TickType_t)(HOLD_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_HOLD_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->doublePressTimer, "Double press timer", (TickType_t)(DOUBLE_PRESS_TIME / portTICK_PERIOD_MS), pdFALSE, BUTTON_DOUBLE_PRESS_TIMEOUT_SIG, &me->super);
    ESP_ERROR_CHECK(gpio_config(&gpioConfig));
}