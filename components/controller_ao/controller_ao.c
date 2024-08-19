#include "controller_ao_priv.h"
#include "controller_menus.h"

static const char *TAG = "MENU";

static void displayMenu(menu_t const * const menu)
{
    printf("\n\n");
    for (int i = 0; i<menu->optionsCount; i++){
        if(i == menu->currentSelect){
            ESP_LOGI(TAG, "-> %s\n", menu->options[i].name);
        } else {
            ESP_LOGI(TAG, "%s\n", menu->options[i].name);
        }
    }
}

static void incrementSelect(menu_t * const menu)
{
    menu->currentSelect++;
    if(menu->currentSelect >= menu->optionsCount){
        menu->currentSelect = 0;
    }
}

State Controller_top(Controller * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0};
    switch (e->sig)
    {
    case ENTRY_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EV_BUTTON_RELEASED:
        incrementSelect(me->menu);
        displayMenu(me->menu);
        evt.sig = CONTROLLER_SELECT_SIG;
        Active_post(&me->super, &evt);
        status = HANDLED_STATUS;
        break;
    
    case EV_BUTTON_HOLD:
        option_t * option = &me->menu->options[me->menu->currentSelect];
        if(option->state != (StateHandler) NULL){
            me->menu = me->menu->options[me->menu->currentSelect].menu;
            status = transition(&me->super.super, option->state);
        } else {
            evt.sig = CONTROLLER_ENTER_SIG;
            Active_post(&me->super, &evt);
            status = HANDLED_STATUS;
        }
        break;
    
    case EV_BUTTON_DOUBLE_PRESS:
        if(me->menu->parentMenu != (menu_t *)NULL){
            evt.sig = CONTROLLER_ESCAPE_SIG;
            Active_post(&me->super, &evt);
            me->menu = me->menu->parentMenu;
            status = transition(&me->super.super, me->menu->originState);
        } else {
            status = HANDLED_STATUS;
        }
        break;
    
    case EXIT_SIG:
        /* code */
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}

State Controller_main(Controller * const me, Event const * const e)
{
    State status;
    // Event evt = { LAST_EVENT_FLAG, (void *)1};
    switch (e->sig)
    {
    case ENTRY_SIG:
        displayMenu(me->menu);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_SELECT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ENTER_SIG:
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ESCAPE_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Controller_top);
        break;
    }
    return status;
}

State Controller_tracking(Controller * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0};
    switch (e->sig)
    {
    case ENTRY_SIG:
        displayMenu(me->menu);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_SELECT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ENTER_SIG:
        Active_post(AO_Broker, &me->menu->options[me->menu->currentSelect].evt);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ESCAPE_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        evt.sig = EV_CONTROLLER_STOP_READING_IMU;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Controller_top);
        break;
    }
    return status;
}

State Controller_calibration(Controller * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0};
    switch (e->sig)
    {
    case ENTRY_SIG:
        displayMenu(me->menu);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_SELECT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ENTER_SIG:
        Active_post(AO_Broker, &me->menu->options[me->menu->currentSelect].evt);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ESCAPE_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EV_IMU_CALIBRATION_DONE:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        evt.sig = EV_CONTROLLER_STOP_CALIBRATION_IMU;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Controller_top);
        break;
    }
    return status;
}

State Controller_connection(Controller * const me, Event const * const e)
{
    State status;
    // Event evt = { LAST_EVENT_FLAG, (void *)0};
    switch (e->sig)
    {
    case ENTRY_SIG:
        displayMenu(me->menu);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_SELECT_SIG:
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ENTER_SIG:
        Active_post(AO_Broker, &me->menu->options[me->menu->currentSelect].evt);
        status = HANDLED_STATUS;
        break;
    
    case CONTROLLER_ESCAPE_SIG:
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Controller_top);
        break;
    };
    return status;
}

State Controller_init(Controller * const me, Event const * const e)
{
    return(transition(&me->super.super, (StateHandler)&Controller_main));
}

void Controller_ctor(Controller * const me)
{
    Active_ctor(&me->super, (StateHandler)&Controller_init);
    me->menu = &mainMenu;
}