#include "events_broker.h"
#include "esp_log.h"

State Broker_publish(Broker * const me, Event const * const e)
{
    if(e->sig >= USER_SIG){
        uint8_t evtId = e->sig-USER_SIG;
        GlobalEvent_t *globalEvent = &me->globalEvents[evtId];
        if(globalEvent->subscribers > 0){
            for (int q = 0; q<globalEvent->subscribers; q++)
            {
                Active *ao=globalEvent->aos[q];
                Active_post(ao, e);
            }
        }
    }
    return HANDLED_STATUS;
}

State Broker_init(Broker * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Broker_publish);
}

void Broker_ctor(Broker * const me)
{
    Active_ctor(&me->super, (StateHandler)&Broker_init);
}

void Broker_subscribe(Broker * const me, Event const * const e, Active * const ao)
{
    uint8_t evtId = e->sig - USER_SIG;
    GlobalEvent_t *globalEvent = &me->globalEvents[evtId];
    assert(globalEvent->subscribers+1<MAX_AOS_PER_EVENT);
    globalEvent->aos[globalEvent->subscribers] = ao;
    globalEvent->subscribers++;
}