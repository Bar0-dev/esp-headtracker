#ifndef IMU_HAL
#define IMU_HAL

#include "core.h"

#define ACCEL_SCALE_FACTOR 1000

#define ACCEL_RANGE_SETTING ACCEL_2G
#define GYRO_RANGE_SETTING GYRO_500DPS
#define MPU_INT_PIN 33

typedef struct {
  uint8_t conf_reg;
  uint8_t config_byte;
} Config_t;

typedef Vector16_t ImuData_t[NO_SENSOR];

void imu_hal_init();
void imu_read(ImuData_t data);
uint8_t imu_get_accel_range();
uint16_t imu_get_gyro_range();
uint16_t imu_get_mag_range();

#endif
