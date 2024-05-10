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
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
 */
static esp_err_t mpu9250_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t mpu9250_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    esp_err_t ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, MPU9250_SENSOR_ADDR, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

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
    imu_who_am_i();
    ESP_ERROR_CHECK(mpu9250_register_write_byte(SMPLRT_DIV, config.sampleDivSetting));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(CONFIG, (config.fifoMode<<FIFO_MODE_OFFSET)|(config.fSyncSetting<<FSYNC_OFFSET)|config.dlpfSetting));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(FIFO_EN, config.fifoEnSetting));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(INT_PIN_CFG, config.intPinCfg));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(INT_ENABLE, config.intPinEnable));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(GYRO_CONFIG, (config.gyroRangeSetting<<GYRO_FS_SEL_OFFSET)|config.fChoiceBSetting));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(ACCEL_CONFIG, config.accelRangeSetting<<ACCEL_FS_SEL_OFFSET));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(ACCEL_CONFIG_2, config.accelDlpfSetting|(config.accelFChoiceBSetting<<ACCEL_FCHOICE_B_OFFSET)));
    ESP_ERROR_CHECK(mpu9250_register_write_byte(PWR_MGMT_1, config.intPinEnable));
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
    ESP_ERROR_CHECK(mpu9250_register_write_byte(PWR_MGMT_1, 1 << DEVICE_RESET));
}

uint8_t imu_who_am_i()
{
    uint8_t data;
    ESP_ERROR_CHECK(mpu9250_register_read(WHO_AM_I, &data, 1));
    ESP_LOGI(TAG, "IMU who am I response: %X", data);
    return data;
}

// AccelDataRaw_t imu_read_accel(void)
// {
//     AccelDataRaw_t data;
//     uint8_t buffer[6];
//     ESP_ERROR_CHECK(mpu9250_register_read(ACCEL_DATA_ADDR, buffer, 6));
//     data.x = (uint16_t)(buffer[ACCEL_XOUT_H]<<8) + buffer[ACCEL_XOUT_L];
//     data.y = (uint16_t)(buffer[ACCEL_YOUT_H]<<8) + buffer[ACCEL_YOUT_L];
//     data.z = (uint16_t)(buffer[ACCEL_ZOUT_H]<<8) + buffer[ACCEL_ZOUT_L];

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

static float map_int16_to_range(int16_t value, int16_t range){
    return ((float)value-INT16_MIN)*2*range/(INT16_MAX-INT16_MIN)-range;
}

static float convert_raw_accel_to_G(int16_t raw){
    AccelRangeConf_t range = conf.accelRangeSetting;
    float calc = 0;

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
    return calc;
}

static float convert_raw_gyro_to_radPerS(int16_t raw){
    GyroRangeConf_t range = conf.gyroRangeSetting;
    float calc = 0;

    switch (range)
    {
    case GYRO_250DPS:
        calc = map_int16_to_range(raw, 250);
        break;
    case GYRO_500DPS:
        calc = map_int16_to_range(raw, 500);
        break;
    case GYRO_1000DPS:
        calc = map_int16_to_range(raw, 1000);
        break;
    case GYRO_2000DPS:
        calc = map_int16_to_range(raw, 2000);
        break;
    default:
        break;
    }
    return calc;
}

ImuDataRaw_t imu_read(void)
{
    uint8_t buffer[14];
    uint8_t interruptSignal = 0;
    while (interruptSignal == 0)
    {
        ESP_ERROR_CHECK(mpu9250_register_read(INT_STATUS, &interruptSignal, 1));
        // ESP_LOGI(TAG, "Interrupt signal: %u", interruptSignal);
    }
    ESP_ERROR_CHECK(mpu9250_register_read(ACCEL_XOUT_H, buffer, 14));
    ImuDataRaw_t data = {
        .accelDataRaw.x = ((int16_t)buffer[ACCEL_XOUT_H_OFFSET] << 8) | buffer[ACCEL_XOUT_L_OFFSET],
        .accelDataRaw.y = ((int16_t)buffer[ACCEL_YOUT_H_OFFSET] << 8) | buffer[ACCEL_YOUT_L_OFFSET],
        .accelDataRaw.z = ((int16_t)buffer[ACCEL_ZOUT_H_OFFSET] << 8) | buffer[ACCEL_ZOUT_L_OFFSET],
        .tempDataRaw = ((int16_t)buffer[TEMP_OUT_H_OFFSET] << 8) | buffer[TEMP_OUT_L_OFFSET],
        .gyroDataRaw.x = ((int16_t)buffer[GYRO_XOUT_H_OFFSET] << 8) | buffer[GYRO_XOUT_L_OFFSET],
        .gyroDataRaw.y = ((int16_t)buffer[GYRO_YOUT_H_OFFSET] << 8) | buffer[GYRO_YOUT_L_OFFSET],
        .gyroDataRaw.z = ((int16_t)buffer[GYRO_ZOUT_H_OFFSET] << 8) | buffer[GYRO_ZOUT_L_OFFSET],
    };
    /**
     * Debug ESP_LOGI
    */
    ESP_LOGI(TAG, "x, y, z:  %.2f   %.2f   %.2f", convert_raw_accel_to_G(data.accelDataRaw.x), convert_raw_accel_to_G(data.accelDataRaw.y), convert_raw_accel_to_G(data.accelDataRaw.z));
    // ESP_LOGI(TAG, "x, y, z:  %.2f   %.2f   %.2f", convert_raw_gyro_to_radPerS(data.gyroDataRaw.x), convert_raw_gyro_to_radPerS(data.gyroDataRaw.y), convert_raw_gyro_to_radPerS(data.gyroDataRaw.z));
    /**
        * Debug ESP_LOGI
    */
    return data;
}