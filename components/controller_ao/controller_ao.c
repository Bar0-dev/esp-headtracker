#include "controller_ao.h"

//Forward declarations
State Controller_top(Controller * const me, Event const * const e);
State Controller_main(Controller * const me, Event const * const e);
State Controller_init(Controller * const me, Event const * const e);
State Controller_tracking(Controller * const me, Event const * const e);
State Controller_calibration(Controller * const me, Event const * const e);
State Controller_connection(Controller * const me, Event const * const e);

static menu_t mainMenu;
static menu_t trackingMenu;
static menu_t calibrationMenu;
static menu_t connectionMenu;

static menu_t mainMenu = {
    .optionsCount = 3,
    .currentSelect = 0,
    .options = {
        {
            .name = "TRACKING",
            .menu = &trackingMenu,
            .state = (StateHandler)&Controller_tracking,
        },
        {
            .name = "CALIBRATION",
            .menu = &calibrationMenu,
            .state = (StateHandler)&Controller_calibration,
        },
        {
            .name = "CONNECTION",
            .menu = &connectionMenu,
            .state = (StateHandler)&Controller_connection,
        },
    },
    .originState = (StateHandler)&Controller_main,
    .parentMenu = (menu_t *)NULL,
};

static menu_t trackingMenu = {
    .optionsCount = 0,
    .currentSelect = 0,
    .options = {},
    .originState = (StateHandler)&Controller_tracking,
    .parentMenu = &mainMenu,
};

static menu_t calibrationMenu = {
    .optionsCount = 3,
    .currentSelect = 0,
    .options = {
        {
            .name = "ACCEL",
            .menu = &mainMenu,
            .state = (StateHandler)&Controller_main,
        },
        {
            .name = "MAG",
            .menu = &mainMenu,
            .state = (StateHandler)&Controller_main,
        },
        {
            .name = "GYRO",
            .menu = &mainMenu,
            .state = (StateHandler)&Controller_main,
        },
    },
    .originState = (StateHandler)&Controller_calibration,
    .parentMenu = &mainMenu,
};

static menu_t connectionMenu = {
    .optionsCount = 2,
    .currentSelect = 0,
    .options = {
        {
            .name = "CONNECT",
            .menu = &mainMenu,
            .state = (StateHandler)&Controller_main,
        },
        {
            .name = "DISCONNECT",
            .menu = &mainMenu,
            .state = (StateHandler)&Controller_main,
        },
    },
    .originState = (StateHandler)&Controller_connection,
    .parentMenu = &mainMenu,
};

static const char *TAG = "CONTROLLER";

static void displayMenu(menu_t const * const menu)
{
    for (int i = 0; i<menu->optionsCount; i++){
        if(i == menu->currentSelect){
            ESP_LOGI(TAG, "*%s\n", menu->options[i].name);
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
    
    case EV_BUTTON_PRESSED:
        incrementSelect(me->menu);
        displayMenu(me->menu);
        evt.sig = CONTROLLER_SELECT_SIG;
        Active_post(&me->super, &evt);
        status = HANDLED_STATUS;
        break;
    
    case EV_BUTTON_HOLD:
        evt.sig = CONTROLLER_ENTER_SIG;
        Active_post(&me->super, &evt);
        me->menu = me->menu->options[me->menu->currentSelect].menu;
        status = transition(&me->super.super, me->menu->options[me->menu->currentSelect].state);
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
    // Event evt = { LAST_EVENT_FLAG, (void *)0}
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
    // Event evt = { LAST_EVENT_FLAG, (void *)0}
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

State Controller_calibration(Controller * const me, Event const * const e)
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