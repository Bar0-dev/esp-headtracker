#include "imu_i2c.h"
#include "esp_err.h"

void mpu_i2c_bus_init(void) {
  int i2c_master_port = I2C_MASTER_NUM;

  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  esp_err_t ret =
      i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE,
                         I2C_MASTER_TX_BUF_DISABLE, 0);
  ESP_ERROR_CHECK(ret);
}

void mpu_i2c_read_bytes(uint8_t device_addr, uint8_t reg_addr, uint8_t *data,
                        size_t len) {
  esp_err_t ret = i2c_master_write_read_device(
      I2C_MASTER_NUM, device_addr, &reg_addr, 1, data, len,
      I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  ESP_ERROR_CHECK(ret);
}

void mpu_i2c_write_byte(uint8_t device_addr, uint8_t reg_addr, uint8_t data) {
  esp_err_t ret;
  uint8_t write_buf[2] = {reg_addr, data};

  ret = i2c_master_write_to_device(I2C_MASTER_NUM, device_addr, write_buf,
                                   sizeof(write_buf),
                                   I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  ESP_ERROR_CHECK(ret);
}

void mpu_i2c_bus_deinit(void) {
  ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
}
