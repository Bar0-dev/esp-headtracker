#include "unity.h"
#include "esp_ao.h"

typedef struct
{
    Active super;
    uint8_t entryCounter;
    uint8_t exitCounter;
    bool transitionReached;
} Mock;

State Mock_init(Mock * const me, Event const * const e);
State Mock_top(Mock * const me, Event const * const e);
State Mock_common(Mock * const me, Event const * const e);
State Mock_substate11(Mock * const me, Event const * const e);
State Mock_substate12(Mock * const me, Event const * const e);
State Mock_substate21(Mock * const me, Event const * const e);
State Mock_substate22(Mock * const me, Event const * const e);

typedef enum {
    TRAN_SIG = USER_SIG,
} MockSigs;

Event tranEvent = { TRAN_SIG, (void*)0};

void Mock_ctor(Mock * const me){
    Active_ctor(&me->super, (StateHandler)&Mock_init);
    me->entryCounter = 0;
    me->exitCounter = 0;
    me->transitionReached = false;
}

State Mock_init(Mock * const me, Event const * const e){
    return transition(&me->super.super, (StateHandler)&Mock_substate12);
}

State Mock_top(Mock * const me, Event const * const e){
    return IGNORED_STATUS;
}

State Mock_common(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_top);
        break;
    }
    return status;
}

State Mock_substate11(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_common);
        break;
    }
    return status;
}

State Mock_substate12(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        break;
    
    case TRAN_SIG:
        status = transition(&me->super.super, (StateHandler)&Mock_substate23);
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate11);
        break;
    }
    return status;
}

State Mock_substate21(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_common);
        break;
    }
    return status;
}

State Mock_substate22(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate21);
        break;
    }
    return status;
}

State Mock_substate23(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->entryCounter++;
        me->transitionReached = true;
        break;
    
    case EXIT_SIG:
        me->exitCounter++;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate22);
        break;
    }
    return status;
}

static Mock mock;
Active *mockAO = &mock.super;

TEST_CASE("Unit test transition from nested state 12 to nested state 23", "espao"){
    Mock_ctor(&mock);
    Active_start(mockAO, "Mock thread", 2048, 1, tskNO_AFFINITY, 10);
    Active_post(&mock.super, &tranEvent);

    TEST_ASSERT(mock.entryCounter == 0);
    TEST_ASSERT(mock.exitCounter == 0);
    TEST_ASSERT(currentParentsMaxIndex == 3);
    for (uint8_t i = 0; i < currentParentsMaxIndex; i++){
        TEST_ASSERT(currentParents[i] == expectedParents[i]);
    }
}