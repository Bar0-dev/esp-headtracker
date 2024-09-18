#include "esp_ao.h"
#include "freertos/timers.h"

#include "esp_log.h"

static Event const entryEvt = {ENTRY_SIG, (void *)0};
static Event const exitEvt = {EXIT_SIG, (void *)0};

static void statesDifference(StateCollection_t *A, StateCollection_t *B,
                             StateCollection_t *out) {
  uint8_t i, j;
  out->length = 0;
  for (i = 0; i < A->length; i++) {
    for (j = 0; j < B->length; j++) {
      if (A->states[i] == B->states[j])
        break;
    }
    if (j == B->length) {
      out->states[out->length] = A->states[i];
      out->length++;
    }
  }
}

static void addStateIfNeeded(StateHandler state, StateCollection_t *states) {
  for (uint8_t i = 0; i < states->length; i++) {
    if (states->states[i] == state) {
      return;
    }
  }
  // State not found in the states array
  states->states[states->length] = state;
  states->length++;
}

static void collectStateHandlers(Hsm *const me, StateHandler state,
                                 StateCollection_t *states) {
  State status;
  states->length = 0;
  states->states[states->length] = state;
  states->length++;
  status = (*state)(me, &(Event){DEFAULT_SIG, (void *)0});
  while (status == SUPER_STATUS) {
    states->states[states->length] = me->parent;
    states->length++;
    status = (*me->parent)(me, &(Event){DEFAULT_SIG, (void *)0});
  }
  return;
}

State Hsm_top(Hsm *const me, Event const *const e) { return IGNORED_STATUS; }

void Hsm_ctor(Hsm *const me, StateHandler initial) {
  me->parent = (StateHandler)0;
  me->state = initial;
}

void Hsm_init(Hsm *const me, Event const *const e) {
  assert(me->state != (StateHandler)0);
  (*me->state)(me, e);
  StateCollection_t entryStates;
  collectStateHandlers(me, me->state, &entryStates);
  for (uint8_t i = 0; i < entryStates.length; i++) {
    (*entryStates.states[i])(me, &entryEvt);
  }
}

void Hsm_dispatch(Hsm *const me, Event const *const e) {
  State status;
  StateHandler prevState = me->state;
  assert(me->state != (StateHandler)0);
  status = (*me->state)(me, e);

  while (status == SUPER_STATUS) {
    status = (*me->parent)(me, e);
  }

  if (status == TRAN_STATUS) {
    // collect the parent states
    StateCollection_t prevStates;
    StateCollection_t targetStates;
    StateCollection_t entryStates;
    StateCollection_t exitStates;
    collectStateHandlers(me, prevState, &prevStates);
    collectStateHandlers(me, me->state, &targetStates);
    // ExitStates
    statesDifference(&prevStates, &targetStates, &exitStates);
    addStateIfNeeded(prevState, &exitStates);
    // EntryStates
    statesDifference(&targetStates, &prevStates, &entryStates);
    addStateIfNeeded(me->state, &entryStates);

    for (uint8_t i = 0; i < exitStates.length; i++) {
      (*exitStates.states[i])(me, &exitEvt);
    }
    for (int8_t i = entryStates.length - 1; i >= 0; i--) {
      (*entryStates.states[i])(me, &entryEvt);
    }
  }
}

void Active_ctor(Active *const me, StateHandler initial) {
  Hsm_ctor(&me->super, initial);
}

static void Active_eventLoop(void *pvParameters) {
  Active *me = (Active *)pvParameters;

  Hsm_init(&me->super, (Event *)0);

  while (1) {
    BaseType_t xReturned;
    Event e;

    xReturned = xQueueReceive(me->queue, (void *)&e, (TickType_t)10);
    if (xReturned == pdPASS) {
      Hsm_dispatch(&me->super, &e);
    }
  }
}

void Active_start(Active *const me, const char *const taskNamePtr,
                  const uint32_t stackSize, /*stack size in bytes*/
                  UBaseType_t taskPriority,
                  const BaseType_t core, /*core the task should run on, default
                                            tskNO_AFFINITY enum*/
                  UBaseType_t queueLength) {
  BaseType_t xReturned;
  assert(me && (taskPriority > 0));

  me->queue = xQueueCreate(queueLength, sizeof(Event));
  assert(me->queue);

  xReturned = xTaskCreatePinnedToCore(&Active_eventLoop, taskNamePtr, stackSize,
                                      me, taskPriority, me->task, core);
  assert(xReturned == pdPASS);
}

void Active_post(Active *const me, Event const *const e) {
  BaseType_t xReturned;
  xReturned = xQueueSend(me->queue, e, (TickType_t)10);
  assert(xReturned == pdPASS);
}

void Active_postFromISR(Active *const me, Event const *const e) {
  BaseType_t xReturned;
  xReturned = xQueueSendFromISR(me->queue, e, (BaseType_t *)pdFALSE);
  assert(xReturned == pdPASS);
}

static TimeEvent *l_timeEvents[MAX_TIMERS];
static uint16_t l_activeTimers = 0;

static void xTimerCallback(TimerHandle_t xTimer) {
  TimeEvent *timeEvent = (TimeEvent *)pvTimerGetTimerID(xTimer);
  assert(xTimer == timeEvent->handle);
  Active_post(timeEvent->act, &timeEvent->super);
}

void TimeEvent_ctor(TimeEvent *const me, char *const timerName,
                    TickType_t period, UBaseType_t autoReload, Signal sig,
                    Active *act) {
  TimerHandle_t xTimer;

  me->super.sig = sig;
  me->act = act;
  xTimer =
      xTimerCreate(timerName, period, autoReload, (void *)0, xTimerCallback);
  assert(xTimer);
  l_timeEvents[l_activeTimers] = me;
  vTimerSetTimerID(xTimer, l_timeEvents[l_activeTimers]);
  l_activeTimers++;
  me->handle = xTimer;
}

void TimeEvent_arm(TimeEvent *const me) {
  BaseType_t xReturned;
  xReturned = xTimerStart(me->handle, (TickType_t)0);
  assert(xReturned);
}

void TimeEvent_disarm(TimeEvent *const me) {
  BaseType_t xReturned;
  xReturned = xTimerStop(me->handle, (TickType_t)0);
  assert(xReturned);
}

void TimeEvent_change_period(TimeEvent *const me, TickType_t period) {
  BaseType_t xReturned;
  xReturned = xTimerChangePeriod(me->handle, period, (TickType_t)0);
  assert(xReturned);
}

void TimeEvent_reset(TimeEvent *const me) {
  BaseType_t xReturned;
  xReturned = xTimerReset(me->handle, (TickType_t)0);
  assert(xReturned);
}
