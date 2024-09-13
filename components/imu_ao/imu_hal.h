#ifndef IMU_HAL
#define IMU_HAL

#include "../registers.h"
#include "core.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"
#include "esp_log.h"

#define I2C_MASTER_SCL_IO 22 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 21 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM                                                         \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces   \
       available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define ACCEL_SCALE_FACTOR 1000

#define ACCEL_RANGE_SETTING ACCEL_2G
#define GYRO_RANGE_SETTING GYRO_500DPS

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
