#include "imu_spi.h"
#include "esp_log.h"
#include <assert.h>

static char *TAG = 'SPI_BUS';

static spi_device_handle_t mpu_spi;

void mpu_spi_bus_init() {
  esp_err_t ret;
  spi_bus_config_t buscfg = {
      .miso_io_num = PIN_NUM_MISO,
      .mosi_io_num = PIN_NUM_MOSI,
      .sclk_io_num = PIN_NUM_CLK,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = SPI_MAX_DMA_LEN,
  };

  spi_device_interface_config_t devcfg = {
      .command_bits = 0,
      .address_bits = 8,
      .dummy_bits = 0,
      .mode = 0,
      .duty_cycle_pos = 128, // default 128 = 50%/50% duty
      .cs_ena_pretrans = 0,  // 0 not used
      .cs_ena_posttrans = 0, // 0 not used
      .clock_speed_hz = 1 * 1000 * 1000,
      .spics_io_num = PIN_NUM_CS,
      .flags = 0, // 0 not used
      .queue_size = 1,
      .pre_cb = NULL,
      .post_cb = NULL,
  };

  // Initialize the SPI bus
  ret = spi_bus_initialize(MPU_SPI_HOST, &buscfg, 0);
  ESP_ERROR_CHECK(ret);

  // Attach the MPU9250 to the SPI bus
  ret = spi_bus_add_device(MPU_SPI_HOST, &devcfg, &mpu_spi);
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "SPI initialized successfully");
}

void mpu_spi_write_bytes(uint8_t regAddr, const uint8_t *data, size_t length) {
  assert(length > 0);
  spi_transaction_t transaction;
  transaction.flags = 0;
  transaction.cmd = 0;
  transaction.addr = (regAddr & SPIBUS_WRITE);
  transaction.length = length * 8;
  transaction.rxlength = 0;
  transaction.user = NULL;
  transaction.tx_buffer = data;
  transaction.rx_buffer = NULL;
  esp_err_t ret = spi_device_transmit(mpu_spi, &transaction);
  ESP_ERROR_CHECK(ret);
}

void mpu_spi_write_byte(uint8_t regAddr, const uint8_t data) {
  mpu_spi_write_bytes(regAddr, &data, 1);
}

void mpu_spi_read_bytes(uint8_t regAddr, uint8_t *data, size_t length) {
  assert(length > 0);
  spi_transaction_t transaction;
  transaction.flags = 0;
  transaction.cmd = 0;
  transaction.addr = (regAddr | SPIBUS_READ);
  transaction.length = length * 8;
  transaction.rxlength = length * 8;
  transaction.user = NULL;
  transaction.tx_buffer = NULL;
  transaction.rx_buffer = data;
  esp_err_t ret = spi_device_transmit(mpu_spi, &transaction);
  ESP_ERROR_CHECK(ret);
}

void mpu_spi_read_byte(uint8_t regAddr, uint8_t *data) {
  mpu_spi_read_bytes(regAddr, data, 1);
}

void mpu_spi_bus_deinit() {
  spi_bus_remove_device(mpu_spi);
  esp_err_t ret = spi_bus_free(MPU_SPI_HOST);
  ESP_ERROR_CHECK(ret);
}
