#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include "esp_ao.h"
#include "events_broker.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "../registers.h"

typedef struct
{
    SampleDiv_t sampleDivSetting;
    FSyncConf_t fSyncSetting;
    DlpfConf_t dlpfSetting;
    FifoModeConf_t fifoMode;
    FifoEn_t fifoEnSetting;
    IntPinCfg_t intPinCfg;
    IntPinEnable_t intPinEnable;
    GyroRangeConf_t gyroRangeSetting;
    FChoiceBConf_t fChoiceBSetting;
    AccelRangeConf_t accelRangeSetting;
    AccelFChoiceBConf_t accelFChoiceBSetting;
    AccelDlpfConf_t accelDlpfSetting;
    PwrMgmt_t pwrMgmtSetting;
    MagControlConf_t magControlSetting;
} ImuConfig_t;


typedef struct 
{
    int16_t x;
    int16_t y;
    int16_t z;
} RawVector_t;

typedef struct 
{
    int32_t x;
    int32_t y;
    int32_t z;
} BufferVector_t;

typedef struct 
{
    float x;
    float y;
    float z;
} Vector_t;

typedef int16_t TempDataRaw_t;

typedef struct
{
    RawVector_t accel;
    RawVector_t gyro;
    RawVector_t mag;
} ImuData_t;

typedef struct
{
    Vector_t accel;
    Vector_t gyro;
    Vector_t mag;
} ImuDataNormalized_t;

typedef enum
{
    NO_SENSOR,
    ACCEL,
    GYRO,
    TEMP,
    MAG
} SensorType_t;

typedef enum
{
    NO_AXIS,
    X_AXIS,
    Y_AXIS,
    Z_AXIS,
} Axis_t;

typedef struct
{
    BufferVector_t accel;
    BufferVector_t gyro;
    BufferVector_t mag;

} CalibrationBuffer_t;


/*
IMU AO code
*/

#define READ_PERIOD 10 //calculation period in ms
#define PRE_CALIBRATION_PERIOD 1*1000 //calculation period in ms (x*1000[ms] = x[s])
#define ACCEL_GYRO_CALIBRATION_PERIOD 3*1000 //calculation period in ms (x*1000[ms] = x[s])
#define MAG_CALIBRATION_PERIOD 10*1000 //calculation period in ms (x*1000[ms] = x[s])

typedef struct
{
    Active super;
    ImuData_t data;
    SensorType_t calibrationSensor;
    Axis_t calibrationAxis;
    uint16_t samples;
    BufferVector_t buffer;
    ImuData_t calibrationOffsets;
    bool calibrationSamplingInProgress;

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