#ifndef IMU_SPI_H
#define IMU_SPI_H

#include "driver/spi_master.h"
#include "registers.h"

#define MPU_SPI_HOST VSPI_HOST
#define PIN_NUM_MISO 19
#define PIN_NUM_MOSI 23
#define PIN_NUM_CLK 18
#define PIN_NUM_CS 5
#define SPIBUS_READ (0x80)
#define SPIBUS_WRITE (0x7F)

spi_device_handle_t *mpu_spi_get_handle();
void mpu_spi_bus_init();
void mpu_spi_write_bytes(uint8_t regAddr, const uint8_t *data, size_t length);
void mpu_spi_write_byte(uint8_t regAddr, uint8_t data);
void mpu_spi_read_bytes(uint8_t regAddr, uint8_t *data, size_t length);
void mpu_spi_read_byte(uint8_t regAddr, uint8_t *data);
void mpu_spi_bus_deinit();

#endif // !IMU_SPI_H
