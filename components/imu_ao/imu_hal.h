#ifndef IMU_HAL
#define IMU_HAL

#include "driver/i2c.h"
#include "../registers.h"
#include "esp_log.h"

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

static const uint8_t accelRange[ACCEL_16G+1] = { 2, 4, 8, 16};
static const uint16_t gyroRange[GYRO_2000DPS+1] = { 250, 500, 1000, 2000 };
static const uint16_t magRange = AK8362_MAX_RANGE;

typedef enum
{
    X_AXIS,
    Y_AXIS,
    Z_AXIS, 
    NO_AXIS
} Axis_t;

typedef enum
{
    ACCEL,
    GYRO,
    MAG,
    NO_SENSOR
} SensorType_t;

typedef enum
{
    X_POS,
    X_NEG,
    Y_POS,
    Y_NEG,
    Z_POS,
    Z_NEG,
    ACCEL_NO_AXIS
} AccelCalibrationAxis_t;

typedef int16_t ImuData_t[NO_SENSOR][NO_AXIS];

void imu_hal_init();
void imu_log_data(ImuData_t data, SensorType_t sensor, bool convert);
void imu_read(ImuData_t data);
void imu_read_accel_axis(int16_t *data, AccelCalibrationAxis_t axis);
void imu_calc_scale_and_bias(int16_t scale[], int16_t bias[], int16_t accel_offsets[]);
void imu_apply_accel_offsets(ImuData_t data, int16_t scales[], int16_t bias[]);

#endif