#include "esp_ao.h"
#include "freertos/timers.h"

#include "esp_log.h"

static Event const entryEvt = { ENTRY_SIG, (void*)0 };
static Event const exitEvt = { EXIT_SIG, (void*)0 };

void Hsm_ctor(Hsm * const me, StateHandler initial)
{
    me->parent = (StateHandler)0;
    me->state = initial;
}

void Hsm_init(Hsm * const me, Event const * const e)
{
    assert(me->state != (StateHandler)0);
    (*me->state)(me, e);
    (*me->state)(me, &entryEvt);
}

uint8_t collectParentStates(Hsm * const me, StateHandler parrentStatesArray[]){
    uint8_t parentIndex = 0;
    State status = SUPER_STATUS;
    while (status != IGNORED_STATUS)
    {
        status = (*me->parent)(me, &(Event){ DEFAULT_SIG, (void*)0 });
        parrentStatesArray[parentIndex] = me->parent;
        parentIndex++;
    }
    return parentIndex;
}

void findCommonParentState(StateHandler currentParents[], StateHandler targetParents[], uint8_t *currentParentIndex, uint8_t *targetParentIndex)
{
    for (uint8_t i = 0; i<*currentParentIndex; i++){
        for (uint8_t j = 0; j<*targetParentIndex; j++){
            if(currentParents[i] == targetParents[j]){
                *currentParentIndex = i;
                *targetParentIndex = j;
                return;
            }
        }
    }
}

void Hsm_dispatch(Hsm * const me, Event const * const e)
{
    State status;
    StateHandler prevState = me->state;
    assert(me->state != (StateHandler)0);
    status = (*me->state)(me ,e);
    
    while (status == SUPER_STATUS)
    {
        status = (*me->parent)(me, e);
    }

    if (status == TRAN_STATUS)
    {
        // collect the parent states
        StateHandler currentParents[MAX_CHILDREN_STATES];
        StateHandler targetParents[MAX_CHILDREN_STATES];

        uint8_t currentParentsMaxIndex = collectParentStates(me, currentParents);
        uint8_t targetParentsMaxIndex = collectParentStates(me, targetParents);
        findCommonParentState(currentParents, targetParents, &currentParentsMaxIndex, &targetParentsMaxIndex);

        (*prevState)(me, &exitEvt);
        for (uint8_t i = 0; i<currentParentsMaxIndex; i++){
            (*currentParents[i])(me, &exitEvt);
        }
        for (int8_t i = targetParentsMaxIndex-1; i>=0; i--){
            (*targetParents[i])(me, &entryEvt);
        }
        status = (*me->state)(me, &entryEvt);
    }
    ESP_LOGV("ESPAO", "\n\nIN DISPATCH\n\n");
}

void Active_ctor(Active * const me, StateHandler initial)
{   
    Hsm_ctor(&me->super, initial);
}

static void Active_eventLoop(void *pvParameters)
{
    Active *me = (Active *)pvParameters;

    Hsm_init(&me->super, (Event *)0);

    while (1)
    {
        BaseType_t xReturned;
        Event e;

        xReturned = xQueueReceive(me->queue, (void *)&e, (TickType_t)10);
        if(xReturned == pdPASS)
        {
            Hsm_dispatch(&me->super, &e);
        }
    }   
}

void Active_start(Active * const me,
                const char *const taskNamePtr,
                const uint32_t stackSize, /*stack size in bytes*/
                UBaseType_t taskPriority,
                const BaseType_t core, /*core the task should run on, default tskNO_AFFINITY enum*/
                UBaseType_t queueLength)
{
    BaseType_t xReturned;
    assert(me && (taskPriority > 0));

    me->queue = xQueueCreate(queueLength, sizeof(Event));
    assert(me->queue);

    xReturned = xTaskCreatePinnedToCore(&Active_eventLoop, taskNamePtr, stackSize, me, taskPriority, me->task, core);
    assert(xReturned == pdPASS);
}

void Active_post(Active * const me, Event const * const e)
{
    BaseType_t xReturned;
    xReturned = xQueueSend(me->queue, e, (TickType_t)10);
    assert(xReturned == pdPASS);
}

static TimeEvent *l_timeEvents[MAX_TIMERS];
static uint16_t l_activeTimers = 0;

static void xTimerCallback(TimerHandle_t xTimer)
{
    TimeEvent *timeEvent = (TimeEvent *)pvTimerGetTimerID(xTimer);
    assert(xTimer == timeEvent->handle);
    Active_post(timeEvent->act, &timeEvent->super);
}

void TimeEvent_ctor(TimeEvent * const me, char * const timerName, TickType_t period, UBaseType_t autoReload, Signal sig, Active *act)
{
    TimerHandle_t xTimer;
    
    me->super.sig = sig;
    me->act = act;
    xTimer = xTimerCreate(timerName, period, autoReload, (void *)0, xTimerCallback);
    assert(xTimer);
    l_timeEvents[l_activeTimers] = me;
    vTimerSetTimerID(xTimer, l_timeEvents[l_activeTimers]);
    l_activeTimers++;
    me->handle = xTimer;
}

void TimeEvent_arm(TimeEvent * const me)
{
    BaseType_t xReturned;
    xReturned = xTimerStart(me->handle, (TickType_t)0);
    assert(xReturned);
}

void TimeEvent_disarm(TimeEvent * const me)
{
    BaseType_t xReturned;
    xReturned = xTimerStop(me->handle, (TickType_t)0);
    assert(xReturned);
}

void TimeEvent_change_period(TimeEvent * const me, TickType_t period)
{
    BaseType_t xReturned;
    xReturned = xTimerChangePeriod(me->handle, period, (TickType_t)0);
    assert(xReturned);
}

void TimeEvent_reset(TimeEvent * const me)
{
    BaseType_t xReturned;
    xReturned = xTimerReset(me->handle, (TickType_t)0);
    assert(xReturned);
}