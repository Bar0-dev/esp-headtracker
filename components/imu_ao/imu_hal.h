#ifndef IMU_HAL
#define IMU_HAL

#include "core.h"
#include "esp_ao.h"
#include <stdint.h>

#define ACCEL_SCALE_FACTOR 1000

#define ACCEL_RANGE_SETTING ACCEL_2G
#define GYRO_RANGE_SETTING GYRO_1000DPS
#define MPU_INT_PIN 26
#define MAX_BUFFER_SIZE 10

// this must be greater than 1; in mpu registers SAMPLE_RATE =
// INTERNAL_SAMPLE_RATE/(1+SAMPLE_DIVIDER_REGISTER_VALUE)
#define SAMPLE_RATE_DIVIDER 2

typedef struct {
  uint8_t conf_reg;
  uint8_t config_byte;
} Config_t;

// TODO: Refactor this to union
typedef Vector16_t ImuData_t[NO_SENSOR];

typedef struct {
  int64_t timeDelta;
  ImuData_t read;
} ImuTimedData_t;

typedef struct {
  ImuTimedData_t data[MAX_BUFFER_SIZE];
  uint8_t length;
} Buffer_t;

typedef struct {
  Buffer_t buffA;
  Buffer_t buffB;
  Buffer_t *writeBuffer;
  Buffer_t *readBuffer;
} DBuffer_t;

void imu_hal_init();
uint8_t imu_hal_get_accel_range();
uint16_t imu_hal_get_gyro_range();
uint16_t imu_hal_get_mag_range();
void imu_hal_enable_interrupt();
void imu_hal_disable_interrupt();

void imu_hal_init_dbuffer();
void imu_hal_update_dbuffer();
Buffer_t *imu_hal_read_buffer();

/**
 * Active objects
 */
extern Active *AO_Imu;

#endif
