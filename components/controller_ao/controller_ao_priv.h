#ifndef CONTROLLER_PRIV_H
#define CONTROLLER_PRIV_H

#include "controller_ao.h"

State Controller_top(Controller * const me, Event const * const e);
State Controller_main(Controller * const me, Event const * const e);
State Controller_init(Controller * const me, Event const * const e);
State Controller_tracking(Controller * const me, Event const * const e);
State Controller_calibration(Controller * const me, Event const * const e);
State Controller_connection(Controller * const me, Event const * const e);

#endif