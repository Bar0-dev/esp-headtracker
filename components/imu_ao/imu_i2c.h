#ifndef IMU_I2C_H
#define IMU_I2C_H

#include "driver/i2c.h"
#include "registers.h"

#define I2C_MASTER_SCL_IO 22 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 21 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM                                                         \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces   \
         available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

void mpu_i2c_bus_init();
void mpu_i2c_write_byte(uint8_t device_addr, uint8_t reg_addr, uint8_t data);
void mpu_i2c_read_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t *data,
                        size_t len);
void mpu_i2c_bus_deinit(void);
#endif // !IMU_I2C_H
