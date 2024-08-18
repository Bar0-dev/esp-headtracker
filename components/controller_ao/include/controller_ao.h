#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "esp_ao.h"
#include "events_broker.h"
#include <esp_log.h>

#define MAX_MENU_OPTIONS 5
#define MAX_OPTION_SIZE 20 

enum ControllerSignals
{
    CONTROLLER_SELECT_SIG = LAST_EVENT_FLAG,
    CONTROLLER_ENTER_SIG,
    CONTROLLER_ESCAPE_SIG,
};

typedef struct menu_t menu_t;

typedef struct
{
    char name[MAX_OPTION_SIZE];
    StateHandler state;
    menu_t * menu;
} option_t ;

typedef uint8_t selection_t;

struct menu_t
{
    uint8_t optionsCount;
    selection_t currentSelect;
    option_t options[MAX_MENU_OPTIONS];
    StateHandler originState;
    menu_t * parentMenu;
};

typedef struct
{
    Active super;
    menu_t *menu;
} Controller;


void Controller_ctor(Controller * const me);

#endif