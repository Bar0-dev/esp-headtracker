#pragma once
#ifndef ESP_AO_H
#define ESO_AO_H

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/timers.h"
#include <stdint.h>

/**
 * Signal definition and reserved signals, USER_SIG is a first signal available
 * for the user
 */

#define MAX_TIMERS 100
#define MAX_CHILDREN_STATES 10

typedef uint16_t Signal;

enum ReservedSignals {
  INIT_SIG,
  ENTRY_SIG,
  EXIT_SIG,
  DEFAULT_SIG,
  USER_SIG,
};

/**
 * Event base class
 */

typedef struct {
  Signal sig;
  void *payload;
  /*additional subclass data*/
} Event;

/**
 * Finite State Machine implementation
 */

typedef struct Hsm Hsm; // Finite state machine struct forward declaration

typedef enum {
  TRAN_STATUS,
  HANDLED_STATUS,
  IGNORED_STATUS,
  INIT_STATUS,
  SUPER_STATUS
} State;

typedef State (*StateHandler)(Hsm *const me, Event const *const e);

struct Hsm {
  StateHandler parent;
  StateHandler state;
};

static inline State transition(Hsm *const me, StateHandler const target) {
  me->state = target;
  return TRAN_STATUS;
}

static inline State super(Hsm *const me, StateHandler const parent) {
  me->parent = parent;
  return SUPER_STATUS;
}

void Hsm_ctor(Hsm *const me, StateHandler initial);
void Hsm_init(Hsm *const me, Event const *const e);
void Hsm_dispatch(Hsm *const me, Event const *const e);
State Hsm_top(Hsm *const me, Event const *const e);

typedef struct {
  uint8_t length;
  StateHandler states[MAX_CHILDREN_STATES];
} StateCollection_t;

/**
 * Active object utils
 */

typedef struct Active Active;

/**
 * Active object base class
 */

struct Active {
  Hsm super;
  TaskHandle_t *task;
  QueueHandle_t queue;
  /*additional subclass data*/
};

void Active_ctor(Active *const me, StateHandler initial);
void Active_start(Active *const me, const char *const taskNamePtr,
                  const uint32_t stackSize, /*stack size in number of bytes*/
                  UBaseType_t taskPriority,
                  const BaseType_t core, /*core the task should run on, default
                                            tskNO_AFFINITY enum*/
                  UBaseType_t queueLength);

void Active_post(Active *const me, Event const *const e);
void Active_postFromISR(Active *const me, Event const *const e);

/**
 * Timer object base class
 */

typedef struct {
  Event super;
  Active *act;
  TickType_t period;
  UBaseType_t autoReload;
  TimerHandle_t handle;
} TimeEvent;

void TimeEvent_ctor(TimeEvent *const me, char *const timerName,
                    TickType_t period, UBaseType_t autoReload, Signal sig,
                    Active *act);           /*associate actor to the timer*/
void TimeEvent_arm(TimeEvent *const me);    /*start xTimer*/
void TimeEvent_disarm(TimeEvent *const me); /*stop xTimer*/
void TimeEvent_change_period(TimeEvent *const me, TickType_t period);
void TimeEvent_reset(TimeEvent *const me);

#endif
