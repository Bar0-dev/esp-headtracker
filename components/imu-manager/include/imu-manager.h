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
    uint16_t x;
    uint16_t y;
    uint16_t z;
} AccelData_t;

typedef struct 
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
} GyroData_t;

typedef uint16_t TempData_t;

typedef struct
{
    AccelData_t accelData;
    TempData_t tempData;
    GyroData_t gyroData;
} ImuData_t;

void imu_init(ImuConfig_t config);
void imu_deinit(void);
void imu_reset(void);
uint8_t imu_who_am_i(void);
ImuData_t imu_read(void);

#endif
