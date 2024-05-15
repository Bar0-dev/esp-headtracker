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
uint8_t imu_who_am_i(uint8_t device_addr);
ImuDataRaw_t imu_read(void);

#endif