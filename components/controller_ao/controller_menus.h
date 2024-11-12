
#ifndef CONTROLLER_MENUS_H
#define CONTROLLER_MENUS_H

#include "controller_ao_priv.h"
#include "events_broker.h"

// TODO: move this to the esp_ao for wider use
const Event emptyEvent = {.sig = LAST_EVENT_FLAG, .payload = (void *)0};
const StateHandler emptyState = (StateHandler)NULL;

menu_t mainMenu;
menu_t trackingMenu;
menu_t calibrationMenu;
menu_t connectionMenu;

menu_t mainMenu = {
    .optionsCount = 3,
    .currentSelect = 0,
    .options =
        {
            {
                .name = "TRACKING",
                .menu = &trackingMenu,
                .state = (StateHandler)&Controller_tracking,
                .evt = emptyEvent,
            },
            {
                .name = "CALIBRATION",
                .menu = &calibrationMenu,
                .state = (StateHandler)&Controller_calibration,
                .evt = emptyEvent,
            },
            {
                .name = "CONNECTION",
                .menu = &connectionMenu,
                .state = (StateHandler)&Controller_connection,
                .evt = emptyEvent,
            },
        },
    .originState = (StateHandler)&Controller_main,
    .parentMenu = (menu_t *)NULL,
};

menu_t trackingMenu = {
    .optionsCount = 2,
    .currentSelect = 0,
    .options =
        {
            {.name = "START",
             .menu = &trackingMenu,
             .state = emptyState,
             .evt = {.sig = EV_CONTROLLER_START_READING_IMU}},
            {.name = "STOP",
             .menu = &trackingMenu,
             .state = emptyState,
             .evt = {.sig = EV_CONTROLLER_STOP_READING_IMU}},
        },
    .originState = (StateHandler)&Controller_tracking,
    .parentMenu = &mainMenu,
};

menu_t calibrationMenu = {
    .optionsCount = 3,
    .currentSelect = 0,
    .options =
        {
            {.name = "ACCEL",
             .menu = &calibrationMenu,
             .state = emptyState,
             .evt = {.sig = EV_CONTROLLER_CALIBRATE_ACCEL}},
            {.name = "MAG",
             .menu = &calibrationMenu,
             .state = emptyState,
             .evt = {.sig = EV_CONTROLLER_CALIBRATE_MAG}},
            {.name = "GYRO",
             .menu = &calibrationMenu,
             .state = emptyState,
             .evt = {.sig = EV_CONTROLLER_CALIBRATE_GYRO}},
        },
    .originState = (StateHandler)&Controller_calibration,
    .parentMenu = &mainMenu,
};

menu_t connectionMenu = {
    .optionsCount = 2,
    .currentSelect = 0,
    .options =
        {
            {
                .name = "CONNECT",
                .menu = &mainMenu,
                .state = emptyState,
                .evt = {.sig = EV_CONTROLLER_CONNECT_DEVICE},
            },
            {
                .name = "DISCONNECT",
                .menu = &mainMenu,
                .state = emptyState,
                .evt = {.sig = EV_CONTROLLER_DISCONNECT_DEVICE},
            },
        },
    .originState = (StateHandler)&Controller_connection,
    .parentMenu = &mainMenu,
};

#endif
