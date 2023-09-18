#include "include/imu-manager.h"

static const char *TAG = "imu-manager";

#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

static ImuConfig_t conf;

/**
 * @brief Read a sequence of bytes from a MPU6050 sensor registers
 */
static esp_err_t mpu6050_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU6050 sensor register
 */
static esp_err_t mpu6050_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    esp_err_t ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
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

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void imu_init(ImuConfig_t config)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_SAMPLE_RATE_DIV_ADDR, config.sampleDivSetting));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_FSYNC_DLPF_CONFIG_ADDR, (config.fSyncSetting<<MPU6050_FSYNC_OFFSET)|config.dlpfSetting));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_FIFO_EN_ADDR, config.fifoEnSetting));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_INT_PIN_CFG_ADDR, config.intPinCfg));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_INT_ENABLE_ADDR, config.intPinEnable));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_GYRO_CONFIG_ADDR, config.gyroRangeSetting<<MPU6050_GYRO_FS_SEL_OFFSET));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_ACCEL_CONFIG_ADDR, config.accelRangeSetting<<MPU6050_ACCEL_FS_SEL_OFFSET));
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_PWR_MGMT_1_ADDR, config.intPinEnable));
    ESP_LOGI(TAG, "IMU initialized successfully");
    conf = config;
}

void imu_deinit(void)
{
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C de-initialized successfully");
}

void imu_reset(void)
{
    ESP_ERROR_CHECK(mpu6050_register_write_byte(MPU6050_PWR_MGMT_1_ADDR, 1 << DEVICE_RESET));
}

uint8_t imu_who_am_i()
{
    uint8_t data;
    ESP_ERROR_CHECK(mpu6050_register_read(MPU6050_WHO_AM_I_REG_ADDR, &data, 1));
    ESP_LOGI(TAG, "IMU who am I response: %X", data);
    return data;
}

// AccelDataRaw_t imu_read_accel(void)
// {
//     AccelDataRaw_t data;
//     uint8_t buffer[6];
//     ESP_ERROR_CHECK(mpu6050_register_read(MPU6050_ACCEL_DATA_ADDR, buffer, 6));
//     data.x = (uint16_t)(buffer[ACCEL_X_OUT_H]<<8) + buffer[ACCEL_X_OUT_L];
//     data.y = (uint16_t)(buffer[ACCEL_Y_OUT_H]<<8) + buffer[ACCEL_Y_OUT_L];
//     data.z = (uint16_t)(buffer[ACCEL_Z_OUT_H]<<8) + buffer[ACCEL_Z_OUT_L];

//     /**
//      * Debug ESP_LOGI
//     */
//     ESP_LOGI(TAG, "Accel read X: %u", data.x);
//     ESP_LOGI(TAG, "Accel read Y: %u", data.y);
//     ESP_LOGI(TAG, "Accel read Z: %u", data.z);
//     /**
//         * Debug ESP_LOGI
//     */
//     return data;
// }

static float map_int16_to_range(int16_t value, uint8_t range){
    
    return (float)value;
}

static float convert_raw_accel_to_G(uint16_t twosComplement){
    AccelConf_t range = conf.accelRangeSetting;
    float calc = 0;
    int16_t raw = (int16_t)twosComplement;

    switch (range)
    {
    case ACCEL_2G:
        calc = map_int16_to_range(raw, 2);
        break;
    case ACCEL_4G:
        calc = map_int16_to_range(raw, 4);
        break;
    case ACCEL_8G:
        calc = map_int16_to_range(raw, 8);
        break;
    case ACCEL_16G:
        calc = map_int16_to_range(raw, 16);
        break;
    default:
        break;
    }
    return raw;
}

ImuDataRaw_t imu_read(void)
{
    uint8_t buffer[14];
    uint8_t interruptSignal = 0;
    while (interruptSignal == 0)
    {
        ESP_ERROR_CHECK(mpu6050_register_read(MPU6050_INT_STATUS_ADDR, &interruptSignal, 1));
        // ESP_LOGI(TAG, "Interrupt signal: %u", interruptSignal);
    }
    ESP_ERROR_CHECK(mpu6050_register_read(MPU6050_ACCEL_DATA_ADDR, buffer, 14));
    ImuDataRaw_t data = {
        .accelDataRaw.x = ((int16_t)buffer[ACCEL_X_OUT_H] << 8) | buffer[ACCEL_X_OUT_L],
        .accelDataRaw.y = ((int16_t)buffer[ACCEL_Y_OUT_H] << 8) | buffer[ACCEL_Y_OUT_L],
        .accelDataRaw.z = ((int16_t)buffer[ACCEL_Z_OUT_H] << 8) | buffer[ACCEL_Z_OUT_L],
        .tempDataRaw = ((int16_t)buffer[TEMP_OUT_H]<<8) | buffer[TEMP_OUT_L],
        .gyroDataRaw.x = ((int16_t)buffer[GYRO_X_OUT_H]<<8) | buffer[GYRO_X_OUT_L],
        .gyroDataRaw.y = ((int16_t)buffer[GYRO_Y_OUT_H]<<8) | buffer[GYRO_Y_OUT_L],
        .gyroDataRaw.z = ((int16_t)buffer[GYRO_Z_OUT_H]<<8) | buffer[GYRO_Z_OUT_L],
    };
    /**
     * Debug ESP_LOGI
    */
    ESP_LOGI(TAG, "x, y, z:  %d   %d   %d", data.accelDataRaw.x, data.accelDataRaw.y, data.accelDataRaw.z);
    // ESP_LOGI(TAG, "Temp read: %hd", data.tempDataRaw);
    /**
        * Debug ESP_LOGI
    */
    return data;
}