#ifndef IMU_H
#define IMU_H

#include <stdio.h>
#include "esp_log.h"
#include "driver/i2c.h"
#include "../registers.c"

typedef struct
{
    SampleDiv_t sampleDivSetting;
    FSyncConf_t fSyncSetting;
    DlpfConf_t dlpfSetting;
    FifoEn_t fifoEnSetting;
    IntPinCfg_t intPinCfg;
    IntPinEnable_t intPinEnable;
    GyroConf_t gyroRangeSetting;
    AccelConf_t accelRangeSetting;
    PwrMgmt_t pwrMgmtSetting;
} ImuConfig_t;

typedef struct 
{
    int16_t x;
    int16_t y;
    int16_t z;
} AccelDataRaw_t;

typedef struct 
{
    int16_t x;
    int16_t y;
    int16_t z;
} GyroDataRaw_t;

typedef int16_t TempDataRaw_t;

typedef struct
{
    AccelDataRaw_t accelDataRaw;
    TempDataRaw_t tempDataRaw;
    GyroDataRaw_t gyroDataRaw;
} ImuDataRaw_t;

typedef struct
{

} GForce_t;


void imu_init(ImuConfig_t config);
void imu_deinit(void);
void imu_reset(void);
uint8_t imu_who_am_i(void);
ImuDataRaw_t imu_read(void);

#endif