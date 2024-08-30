#ifndef IMU_HAL
#define IMU_HAL

#include "driver/i2c.h"
#include "../registers.h"
#include "esp_log.h"
#include "core.h"

#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

#define ACCEL_SCALE_FACTOR 1000

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

typedef Vector16_t ImuData_t[NO_SENSOR];

void imu_hal_init();
void imu_read(ImuData_t data);
uint8_t imu_get_accel_range();
uint16_t imu_get_gyro_range();
uint16_t imu_get_mag_range();

#endif