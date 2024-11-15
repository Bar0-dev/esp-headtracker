#include "coms_ao.h"
#include "core.h"
#include "events_broker.h"
#include <bt_stack.h>
#include <stdint.h>
#include <string.h>

// Forward declarations
State Coms_init(Coms *const me, Event const *const e);
State Coms_idle(Coms *const me, Event const *const e);

State Coms_init(Coms *const me, Event const *const e) {
  return transition(&me->super.super, (StateHandler)&Coms_idle);
}

State Coms_idle(Coms *const me, Event const *const e) {
  State status;
  Event evt = {LAST_EVENT_FLAG, (void *)0};

  switch (e->sig) {
  case ENTRY_SIG:
    evt.sig = EV_CONTROLLER_CONNECT_DEVICE;
    Active_post(&me->super, &evt);
    status = HANDLED_STATUS;
    break;

  case EV_CONTROLLER_CONNECT_DEVICE:
    bt_stack_init();
    evt.sig = BT_CONNECTED_SIG;
    Active_post(&me->super, &evt);
    status = HANDLED_STATUS;
    break;

  case EV_CONTROLLER_DISCONNECT_DEVICE:
    char msg[] = "TEST";
    bt_stack_write((uint8_t *)msg, sizeof(msg));
    status = HANDLED_STATUS;
    break;

  case BT_CONNECTED_SIG:
    status = HANDLED_STATUS;
    break;

  case EV_IMU_SEND_DATA:
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

void Coms_ctor(Coms *const me) {
  Active_ctor(&me->super, (StateHandler)&Coms_init);
}
