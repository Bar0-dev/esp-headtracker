#include "imu_hal.h"
#include "esp_err.h"
#include "imu_ao.h"
#include <assert.h>
#include <stdint.h>

static const Config_t imu_conf[] = {
    {SMPLRT_DIV, 0},
    {CONFIG, (ALLOW_OVERFLOW << FIFO_MODE) | (FSYNC_DISABLED << EXT_SYNC_SET) |
                 (DLPF_20Hz << DLPF_CFG)},
    {GYRO_CONFIG, (GYRO_FCHOICE_B_DISABLED << GYRO_FCHOICE_B) |
                      (GYRO_RANGE_SETTING << GYRO_FS_SEL) |
                      (0 << ZGYRO_SELF_TEST) | (0 << YGYRO_SELF_TEST) |
                      (0 << XGYRO_SELF_TEST)},
    {ACCEL_CONFIG, (ACCEL_RANGE_SETTING << ACCEL_FS_SEL) |
                       (0 << ACCEL_SELF_TEST_X) | (0 << ACCEL_SELF_TEST_Y) |
                       (0 << ACCEL_SELF_TEST_Z)},
    {ACCEL_CONFIG_2, (ACCEL_FCHOICE_B_DISABLED << ACCEL_FCHOICE_B) |
                         (ACCEL_DLPF_21p2Hz << A_DLPFCFG)},
    {LP_ACCEL_ODR, 0},
    {FIFO_EN, 0},
    {WOM_THR, 0},
    {I2C_MST_CTRL, (1 << I2C_SLV0_RNW) | (AK8362_SENSOR_ADDR)},
    {I2C_SLVO_ADDR, AK8362_STATUS_1},
    {I2C_SLVO_REG, 5},
    {I2C_SLVO_CTRL, (1 << I2C_SLV0_EN) | (1 << I2C_SLV0_BYTE_SW) |
                        (1 << I2C_SLV0_REG_DIS) | (1 << I2C_SLV0_GRP) |
                        (5 << I2C_SLV0_LENG)},
    {INT_PIN_CFG, (0 << ACTL) | (0 << INT_OPEN) | (0 << LATCH_INT_EN) |
                      (1 << INT_ANYRD_2CLEAR) | (0 << ACTL_FSYNC) |
                      (0 << FSYNC_INT_MODE_EN) | (1 << I2C_BYPASS_EN)},
    {INT_ENABLE, (1 << DATA_RDY_EN)},
    {MOT_DETECT_CTRL, 0},
    {USER_CTRL, 0},
    {PWR_MGMT_1, 0},
    {PWR_MGMT_2, 0}};

static const uint8_t accelRange[ACCEL_16G + 1] = {2, 4, 8, 16};
static const uint16_t gyroRange[GYRO_2000DPS + 1] = {250, 500, 1000, 2000};
static const uint16_t magRange = AK8362_MAX_RANGE;

/**
 * @brief Read a accelCalAxis of bytes from a MPU9250 sensor registers
 */
static esp_err_t imu_register_read(uint8_t device_addr, uint8_t reg_addr,
                                   uint8_t *data, size_t len) {
  return i2c_master_write_read_device(
      I2C_MASTER_NUM, device_addr, &reg_addr, 1, data, len,
      I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t imu_register_write_byte(uint8_t device_addr, uint8_t reg_addr,
                                         uint8_t data) {
  esp_err_t ret;
  uint8_t write_buf[2] = {reg_addr, data};

  ret = i2c_master_write_to_device(I2C_MASTER_NUM, device_addr, write_buf,
                                   sizeof(write_buf),
                                   I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

  return ret;
}

/**
 * @brief i2c master initialization
 */
static void i2c_master_init(void) {
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

void imu_config() {
  for (uint8_t index = 0; index < sizeof(imu_conf) / sizeof(Config_t);
       index++) {
    esp_err_t ret =
        imu_register_write_byte(MPU9250_SENSOR_ADDR, imu_conf[index].conf_reg,
                                imu_conf[index].config_byte);
    ESP_ERROR_CHECK(ret);
  }
  uint8_t readSetting;
  for (uint8_t index = 0; index < sizeof(imu_conf) / sizeof(Config_t);
       index++) {
    esp_err_t ret = imu_register_read(
        MPU9250_SENSOR_ADDR, imu_conf[index].conf_reg, &readSetting, 1);
    ESP_ERROR_CHECK(ret);
    assert(readSetting == imu_conf[index].config_byte);
  }
}

void imu_hal_init() {
  i2c_master_init();
  imu_config();
}

// UNUSED FUNC
//  static void imu_deinit(void)
//  {
//      ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
//  }

// static void imu_reset(void)
// {
//     ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, PWR_MGMT_1,
//     1 << DEVICE_RESET));
// }

// static uint8_t imu_who_am_i(uint8_t device_addr)
// {
//     uint8_t data;
//     esp_err_t ret;
//     if(device_addr == MPU9250_SENSOR_ADDR){
//         ret = imu_register_read(device_addr, WHO_AM_I, &data, 1);
//     } else {
//         ret = imu_register_read(device_addr, AK8362_WHO_AM_I, &data, 1);
//     }
//     ESP_ERROR_CHECK(ret);
//     return data;
// }

static void mag_read(ImuData_t data) {
  uint8_t buffer[6];
  uint8_t overflow = 0;
  ESP_ERROR_CHECK(
      imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_1, &overflow, 1));
  ESP_ERROR_CHECK(
      imu_register_read(AK8362_SENSOR_ADDR, AK8362_MAG_DATA, buffer, 6));
  data[MAG][X_AXIS] =
      ((int16_t)buffer[MAG_XOUT_H_OFFSET] << 8) | buffer[MAG_XOUT_L_OFFSET];
  data[MAG][Y_AXIS] =
      ((int16_t)buffer[MAG_YOUT_H_OFFSET] << 8) | buffer[MAG_YOUT_L_OFFSET];
  data[MAG][Z_AXIS] =
      ((int16_t)buffer[MAG_ZOUT_H_OFFSET] << 8) | buffer[MAG_ZOUT_L_OFFSET];
  ESP_ERROR_CHECK(
      imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_2, &overflow, 1));
}

void imu_read(ImuData_t data) {
  DataOffsets_t offset = ACCEL_XOUT_H_OFFSET;
  uint8_t bufferSize = GYRO_ZOUT_L_OFFSET + 1;
  uint8_t buffer[bufferSize];
  ESP_ERROR_CHECK(
      imu_register_read(MPU9250_SENSOR_ADDR, ACCEL_XOUT_H, buffer, bufferSize));
  for (uint8_t sensor = ACCEL; sensor <= GYRO; sensor++) {
    if (sensor == GYRO) {
      offset = GYRO_XOUT_H_OFFSET;
    }
    for (uint8_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      data[sensor][axis] = ((int16_t)buffer[2 * axis + offset] << 8) |
                           buffer[2 * axis + offset + 1];
    }
  }
  mag_read(data);
  return;
}

uint8_t imu_get_accel_range() { return accelRange[ACCEL_RANGE_SETTING]; }

uint16_t imu_get_gyro_range() { return gyroRange[GYRO_RANGE_SETTING]; }

uint16_t imu_get_mag_range() { return magRange; }
