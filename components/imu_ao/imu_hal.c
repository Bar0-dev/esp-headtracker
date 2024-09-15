#include "imu_hal.h"
#include "esp_log.h"
#include "imu_i2c.h"
#include "imu_spi.h"
#include "registers.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

const char *TAG = "IMU_HAL";

static const Config_t mpu_conf_1[] = {
    {PWR_MGMT_1, 1 << DEVICE_RESET},
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
    {I2C_MST_CTRL, 0},
    {I2C_SLVO_ADDR, (1 << I2C_SLV0_RNW) | (AK8362_SENSOR_ADDR)},
    {I2C_SLVO_REG, AK8362_MAG_DATA},
    {I2C_SLVO_CTRL, (1 << I2C_SLV0_EN) | (1 << I2C_SLV0_BYTE_SW) |
                        (0 << I2C_SLV0_REG_DIS) | (0 << I2C_SLV0_GRP) |
                        (7 << I2C_SLV0_LENG)},
    {INT_PIN_CFG, (0 << ACTL) | (0 << INT_OPEN) | (0 << LATCH_INT_EN) |
                      (1 << INT_ANYRD_2CLEAR) | (0 << ACTL_FSYNC) |
                      (0 << FSYNC_INT_MODE_EN) | (1 << I2C_BYPASS_EN)},
    {INT_ENABLE, (1 << DATA_RDY_EN)},
    {MOT_DETECT_CTRL, 0},
    {USER_CTRL, (0 << I2C_MST_EN) | (0 << I2C_IF_DIS)},
    {PWR_MGMT_1, 0},
    {PWR_MGMT_2, 0}};

static const Config_t mag_conf[] = {
    {AK8362_CONTROL_1,
     (COUNTINIOUS_MODE_2 << MAG_OUTPUT_MODE) | (1 << MAG_OUTPUT_WIDTH)}};

static const Config_t mpu_conf_2[] = {
    {INT_PIN_CFG, (0 << I2C_BYPASS_EN)},
    {I2C_MST_CTRL, I2C_MST_CLK_400kHz},
    {USER_CTRL, (1 << I2C_MST_EN) | (1 << I2C_IF_DIS)},
};

static const uint8_t accelRange[ACCEL_16G + 1] = {2, 4, 8, 16};
static const uint16_t gyroRange[GYRO_2000DPS + 1] = {250, 500, 1000, 2000};

static void spi_set_and_check_config_arr(Config_t const confArr[],
                                         uint8_t arrsize) {
  mpu_spi_bus_init();
  for (uint8_t index = 0; index < arrsize / sizeof(Config_t); index++) {
    mpu_spi_write_byte(confArr[index].conf_reg, confArr[index].config_byte);
    ESP_LOGI(TAG, "i:%d, reg:%d, set:%d", index, confArr[index].conf_reg,
             confArr[index].config_byte);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  uint8_t readSetting;
  for (uint8_t index = 0; index < arrsize / sizeof(Config_t); index++) {
    if (confArr[index].conf_reg == PWR_MGMT_1) {
      continue;
    }
    mpu_spi_read_byte(confArr[index].conf_reg, &readSetting);
    ESP_LOGI(TAG, "i:%d, reg:%d, set:%d", index, confArr[index].conf_reg,
             readSetting);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    assert(readSetting == confArr[index].config_byte);
  }
  vTaskDelay(100 / portTICK_PERIOD_MS);
  mpu_spi_bus_deinit();
}

static void i2c_set_and_check_config_arr(uint8_t sensorAddr,
                                         Config_t const confArr[],
                                         uint8_t arrsize) {
  mpu_i2c_bus_init();
  for (uint8_t index = 0; index < arrsize / sizeof(Config_t); index++) {
    mpu_i2c_write_byte(sensorAddr, confArr[index].conf_reg,
                       confArr[index].config_byte);
    ESP_LOGI(TAG, "i:%d, reg:%d, set:%d", index, confArr[index].conf_reg,
             confArr[index].config_byte);
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
  uint8_t readSetting;
  for (uint8_t index = 0; index < arrsize / sizeof(Config_t); index++) {
    if (confArr[index].conf_reg == PWR_MGMT_1 ||
        confArr[index].conf_reg == USER_CTRL) {
      continue;
    }
    mpu_i2c_read_bytes(sensorAddr, confArr[index].conf_reg, &readSetting, 1);
    ESP_LOGI(TAG, "i:%d, reg:%d, set:%d", index, confArr[index].conf_reg,
             readSetting);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    assert(readSetting == confArr[index].config_byte);
  }
  mpu_i2c_bus_deinit();
}

void imu_hal_init() {
  spi_set_and_check_config_arr(mpu_conf_1, sizeof(mpu_conf_1));
  i2c_set_and_check_config_arr(AK8362_SENSOR_ADDR, mag_conf, sizeof(mag_conf));
  spi_set_and_check_config_arr(mpu_conf_2, sizeof(mpu_conf_2));
  mpu_spi_bus_init();
}

// static void mag_read(ImuData_t data) {
//   uint8_t buffer[6];
//   uint8_t overflow = 0;
//   ESP_ERROR_CHECK(
//       imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_1, &overflow, 1));
//   ESP_ERROR_CHECK(
//       imu_register_read(AK8362_SENSOR_ADDR, AK8362_MAG_DATA, buffer, 6));
//   data[MAG][X_AXIS] =
//       ((int16_t)buffer[MAG_XOUT_H_OFFSET] << 8) | buffer[MAG_XOUT_L_OFFSET];
//   data[MAG][Y_AXIS] =
//       ((int16_t)buffer[MAG_YOUT_H_OFFSET] << 8) | buffer[MAG_YOUT_L_OFFSET];
//   data[MAG][Z_AXIS] =
//       ((int16_t)buffer[MAG_ZOUT_H_OFFSET] << 8) | buffer[MAG_ZOUT_L_OFFSET];
//   ESP_ERROR_CHECK(
//       imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_2, &overflow, 1));
// }
//
// void imu_read(ImuData_t data) {
//   DataOffsets_t offset = ACCEL_XOUT_H_OFFSET;
//   uint8_t bufferSize = GYRO_ZOUT_L_OFFSET + 1;
//   uint8_t buffer[bufferSize];
//   ESP_ERROR_CHECK(
//       imu_register_read(MPU9250_SENSOR_ADDR, ACCEL_XOUT_H, buffer,
//       bufferSize));
//   for (uint8_t sensor = ACCEL; sensor <= GYRO; sensor++) {
//     if (sensor == GYRO) {
//       offset = GYRO_XOUT_H_OFFSET;
//     }
//     for (uint8_t axis = X_AXIS; axis < NO_AXIS; axis++) {
//       data[sensor][axis] = ((int16_t)buffer[2 * axis + offset] << 8) |
//                            buffer[2 * axis + offset + 1];
//     }
//   }
//   mag_read(data);
//   return;
// }

uint8_t imu_get_accel_range() { return accelRange[ACCEL_RANGE_SETTING]; }

uint16_t imu_get_gyro_range() { return gyroRange[GYRO_RANGE_SETTING]; }

uint16_t imu_get_mag_range() { return AK8362_MAX_RANGE; }
