#ifndef IMU_H
#define IMU_H

#include "../imu_hal.h"
#include "../imu_helpers.h"
#include "esp_ao.h"
#include "events_broker.h"
#include "nvs_helpers.h"
#include <stdio.h>

#define SEND_ORIENTATION_PERIOD 20 // send pacekt period in ms
#define PRE_CALIBRATION_PERIOD                                                 \
  5 * 1000 // calculation period in ms (x*1000[ms] = x[s])
#define ACCEL_GYRO_CALIBRATION_PERIOD                                          \
  1 * 1000 // calculation period in ms (x*1000[ms] = x[s])
#define MAG_CALIBRATION_PERIOD                                                 \
  10 * 1000 // calculation period in ms (x*1000[ms] = x[s])
#define SAMPLE_RATE 1000 / SAMPLE_RATE_DIVIDER

typedef struct {
  Active super;
  CalibtrationData_t calibration;
  TimeEvent calibrationTimer;
  TimeEvent preCalibrationTimer;
  TimeEvent orientationSendTimer;
} Imu;

enum ImuEventSignals {
  IMU_CALIBRATION_TIMEOUT_SIG = LAST_EVENT_FLAG,
  IMU_PRE_CALIBRATION_TIMEOUT_SIG,
  IMU_AXIS_CAL_DONE,
  IMU_ORIENTATION_PACKET_SEND_TIMEOUT_SIG,
};

void Imu_ctor(Imu *const me);

/**
 * Active objects
 */
extern Active *AO_Broker;

#endif
