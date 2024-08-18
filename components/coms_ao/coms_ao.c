#include "coms_ao.h"

//Forward declarations
State Coms_init(Coms * const me, Event const * const e);
State Coms_idle(Coms * const me, Event const * const e);


State Coms_init(Coms * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Coms_idle);
}

State Coms_idle(Coms * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        prov_mgr_init();
        break;

    case EXIT_SIG:
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}

void Coms_ctor(Coms * const me){
    Active_ctor(&me->super, (StateHandler)&Coms_init);
}