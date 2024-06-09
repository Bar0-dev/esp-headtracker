#include "unity.h"
#include "esp_ao.h"
#include "esp_log.h"

typedef struct
{
    Active super;
    bool transitionReached;
    bool tranSigReceived;

    bool commonStateEntered;
    bool commonStateExited;
    bool substate11StateEntered;
    bool substate11StateExited;
    bool substate12StateEntered;
    bool substate12StateExited;
    bool substate13StateEntered;
    bool substate13StateExited;
    bool substate21StateEntered;
    bool substate21StateExited;
    bool substate22StateEntered;
    bool substate22StateExited;
    bool substate23StateEntered;
    bool substate23StateExited;
} Mock;

State Mock_init(Mock * const me, Event const * const e);
State Mock_top(Mock * const me, Event const * const e);
State Mock_common(Mock * const me, Event const * const e);
State Mock_substate11(Mock * const me, Event const * const e);
State Mock_substate12(Mock * const me, Event const * const e);
State Mock_substate13(Mock * const me, Event const * const e);
State Mock_substate21(Mock * const me, Event const * const e);
State Mock_substate22(Mock * const me, Event const * const e);
State Mock_substate23(Mock * const me, Event const * const e);

typedef enum {
    TRAN_SIG = USER_SIG,
    TRAN2_SIG,
    TRAN3_SIG,
    TRAN4_SIG,
    TRAN5_SIG,
} MockSigs;

void reset_flags(Mock * const me){
    me->commonStateEntered = false;
    me->commonStateExited = false;
    me->substate11StateEntered = false;
    me->substate11StateExited = false;
    me->substate12StateEntered = false;
    me->substate12StateExited = false;
    me->substate13StateEntered = false;
    me->substate13StateExited = false;
    me->substate21StateEntered = false;
    me->substate21StateExited = false;
    me->substate22StateEntered = false;
    me->substate22StateExited = false;
    me->substate23StateEntered = false;
    me->substate23StateExited = false;
    me->transitionReached = false;
    me->tranSigReceived = false;
}

void Mock_ctor(Mock * const me){
    Active_ctor(&me->super, (StateHandler)&Mock_init);
    reset_flags(me);
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
        me->commonStateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        me->commonStateExited = true;
        status = HANDLED_STATUS;
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
        me->substate11StateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case TRAN3_SIG:
        me->tranSigReceived = true;
        status = transition(&me->super.super, (StateHandler)&Mock_substate13);
        break;

    case TRAN5_SIG:
        me->tranSigReceived = true;
        status = transition(&me->super.super, (StateHandler)&Mock_substate12);
        break;
    
    case EXIT_SIG:
        me->substate11StateExited = true;
        status = HANDLED_STATUS;
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
        me->substate12StateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case TRAN_SIG:
        me->tranSigReceived = true;
        status = transition(&me->super.super, (StateHandler)&Mock_substate23);
        break;
    
    case EXIT_SIG:
        me->substate12StateExited = true;
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate11);
        break;
    }
    return status;
}

State Mock_substate13(Mock * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        me->substate13StateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case TRAN4_SIG:
        me->tranSigReceived = true;
        status = transition(&me->super.super, (StateHandler)&Mock_substate11);
        break;
    
    case EXIT_SIG:
        me->substate13StateExited = true;
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate12);
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
        me->substate21StateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        me->substate21StateExited = true;
        status = HANDLED_STATUS;
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
        me->substate22StateEntered = true;
        status = HANDLED_STATUS;
        break;
    
    case EXIT_SIG:
        me->substate22StateExited = true;
        status = HANDLED_STATUS;
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
        me->substate23StateEntered = true;
        me->transitionReached = true;
        status = HANDLED_STATUS;
        break;
    
    case TRAN2_SIG:
        me->tranSigReceived = true;
        status = transition(&me->super.super, (StateHandler)&Mock_substate11);
        break;

    case EXIT_SIG:
        me->substate23StateExited = true;
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Mock_substate22);
        break;
    }
    return status;
}

void testWait(uint16_t delayInTicks){
    int i = 0;
    while(i<delayInTicks){
        printf(" ");
        i++;
    }
    ESP_LOGI("TEST DONE WAITING", "%d", i);
}

static Mock mock;
Active *mockAO = &mock.super;

TEST_CASE("AO with HSM creation", "espao HSM"){
    Mock_ctor(&mock);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_init, "Mock HSM is not in the init state!");
}

TEST_CASE("AO HSM startup and transition from init to entry substate12", "espao HSM"){
    Active_start(mockAO, "Mock thread", 2048, 1, tskNO_AFFINITY, 20);
    testWait(200U);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate12, "Mock HSM is not in the substate12!");
    TEST_ASSERT_MESSAGE(mock.commonStateEntered == true, "CommonState was not enetred!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == true, "State11 was not enetred!");
    TEST_ASSERT_MESSAGE(mock.substate12StateEntered == true, "State12 was not enetred!");
}

TEST_CASE("AO HSM transition from nested 12 to nested 23", "espao HSM"){
    //Reset common entry and exit flags to check if they are changing on transiton from nested to nested
    reset_flags(&mock);
    Event tranEvent = { TRAN_SIG, (void*)0};
    Active_post(mockAO, &tranEvent);
    testWait(200U);
    TEST_ASSERT(mock.tranSigReceived == true);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate23, "Mock HSM is not in the substate23!");
    TEST_ASSERT_MESSAGE(mock.commonStateExited == false, "Common was exited!");
    TEST_ASSERT_MESSAGE(mock.substate12StateExited == true, "State12 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate12StateEntered == false, "State12 entered!");
    TEST_ASSERT_MESSAGE(mock.substate11StateExited == true, "State11 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == false, "State11 entered!");
    TEST_ASSERT_MESSAGE(mock.commonStateEntered == false, "Common was enetred!");
    TEST_ASSERT_MESSAGE(mock.substate21StateEntered == true, "State21 was not enetred!");
    TEST_ASSERT_MESSAGE(mock.substate21StateExited == false, "State21 exited!");
    TEST_ASSERT_MESSAGE(mock.substate22StateEntered == true, "State22 was not enetred!");
    TEST_ASSERT_MESSAGE(mock.substate22StateExited == false, "State22 exited!");
    TEST_ASSERT_MESSAGE(mock.substate23StateEntered == true, "State23 was not enetred!");
    TEST_ASSERT_MESSAGE(mock.substate23StateExited == false, "State23 exited!");
    TEST_ASSERT(mock.transitionReached == true);
}

TEST_CASE("AO HSM transition from nested 23 to nested 11 with EVENT bubbling to nested 21", "espao HSM"){
    reset_flags(&mock);
    Event tran2Event = { TRAN2_SIG, (void*)0};
    Active_post(mockAO, &tran2Event);
    testWait(200U);
    TEST_ASSERT(mock.tranSigReceived == true);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate11, "Mock HSM is not in the substate11!");
    TEST_ASSERT_MESSAGE(mock.commonStateExited == false, "Common was exited!");
    TEST_ASSERT_MESSAGE(mock.substate23StateExited == true, "State23 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate22StateExited == true, "State22 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate21StateExited == true, "State21 was not exited!");
    TEST_ASSERT_MESSAGE(mock.commonStateEntered == false, "Common was enetred!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == true, "State11 was not enetred!");
}

TEST_CASE("AO HSM transition from nested 11 to nested 13", "espao HSM"){
    reset_flags(&mock);
    Event tran3Event = { TRAN3_SIG, (void*)0};
    Active_post(mockAO, &tran3Event);
    testWait(200U);
    TEST_ASSERT(mock.tranSigReceived == true);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate13, "Mock HSM is not in the substate13!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == false, "State11 was entered!");
    TEST_ASSERT_MESSAGE(mock.substate11StateExited == true, "State11 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate12StateEntered == true, "State12 was not entered!");
    TEST_ASSERT_MESSAGE(mock.substate12StateExited == false, "State12 was exited!");
    TEST_ASSERT_MESSAGE(mock.substate13StateEntered == true, "State13 was not entered!");
    TEST_ASSERT_MESSAGE(mock.substate13StateExited == false, "State13 was exited!");
    TEST_ASSERT_MESSAGE(mock.commonStateEntered == false, "Common was enetred!");
}

TEST_CASE("AO HSM transition from nested 13 to nested 11", "espao HSM"){
    reset_flags(&mock);
    Event tran4Event = { TRAN4_SIG, (void*)0};
    Active_post(mockAO, &tran4Event);
    testWait(200U);
    TEST_ASSERT(mock.tranSigReceived == true);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate11, "Mock HSM is not in the substate11!");
    TEST_ASSERT_MESSAGE(mock.substate13StateEntered == false, "State13 was entered!");
    TEST_ASSERT_MESSAGE(mock.substate13StateExited == true, "State13 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate12StateEntered == false, "State12 entered!");
    TEST_ASSERT_MESSAGE(mock.substate12StateExited == true, "State12 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == true, "State11 was not entered!");
    TEST_ASSERT_MESSAGE(mock.substate11StateExited == false, "State11 was exited!");
}

TEST_CASE("AO HSM transition from nested 11 to nested 12", "espao HSM"){
    reset_flags(&mock);
    Event tran5Event = { TRAN5_SIG, (void*)0};
    Active_post(mockAO, &tran5Event);
    testWait(200U);
    TEST_ASSERT(mock.tranSigReceived == true);
    TEST_ASSERT_MESSAGE(mock.super.super.state == (StateHandler)&Mock_substate12, "Mock HSM is not in the substate11!");
    TEST_ASSERT_MESSAGE(mock.substate11StateEntered == false, "State11 was entered!");
    TEST_ASSERT_MESSAGE(mock.substate11StateExited == true, "State11 was not exited!");
    TEST_ASSERT_MESSAGE(mock.substate12StateEntered == true, "State12 not entered!");
    TEST_ASSERT_MESSAGE(mock.substate12StateExited == false, "State12 was exited!");
}