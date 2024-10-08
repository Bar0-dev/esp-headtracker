#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include "esp_ao.h"
#include "events_broker.h"
#include "nvs_helpers.h"
#include "../imu_hal.h"
#include "../imu_helpers.h"

#define READ_PERIOD 10 //calculation period in ms
#define PRE_CALIBRATION_PERIOD 5*1000 //calculation period in ms (x*1000[ms] = x[s])
#define ACCEL_GYRO_CALIBRATION_PERIOD 1*1000 //calculation period in ms (x*1000[ms] = x[s])
#define MAG_CALIBRATION_PERIOD 10*1000 //calculation period in ms (x*1000[ms] = x[s])

typedef struct
{
    Active super;
    CalibtrationData_t calibration;
    TimeEvent readTimer;
    TimeEvent calibrationTimer;
    TimeEvent preCalibrationTimer;
} Imu;

enum ImuEventSignals
{
    IMU_READ_TIMEOUT_SIG = LAST_EVENT_FLAG,
    IMU_CALIBRATION_TIMEOUT_SIG,
    IMU_PRE_CALIBRATION_TIMEOUT_SIG,
    IMU_AXIS_CAL_DONE,
};

void Imu_ctor(Imu * const me);

/**
 * Active objects
*/
extern Active *AO_Broker;

#endif