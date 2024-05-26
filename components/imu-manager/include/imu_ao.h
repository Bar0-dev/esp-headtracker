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

typedef enum
{
    ACCEL,
    GYRO,
    TEMP,
    MAG
} SensorType_t;

/*
IMU AO code
*/

#define POSITION_CALCULATION_PERIOD 10 //calculation period in ms

typedef struct
{
    Active super;
    TimeEvent positionCalculationLoopTimer;
} Imu;

enum ImuEventSignals
{
    CALCULATE_POSITION_TIMER_SIG = LAST_EVENT_FLAG,
};

void Imu_ctor(Imu * const me);

#endif