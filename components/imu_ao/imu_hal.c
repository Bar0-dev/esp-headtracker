#include "imu_hal.h"

static const ImuConfig_t imu_conf= {
    .sampleDivSetting = 0,
    .fSyncSetting = FSYNC_DISABLED,
    .dlpfSetting = DLPF_10Hz,
    .fifoMode = ALLOW_OVERFLOW,
    .fifoEnSetting =
        0<<SLV0_FIFO_EN|\
        0<<SLV1_FIFO_EN|\
        0<<SLV2_FIFO_EN|\
        0<<ACCEL_FIFO_EN|\
        0<<ZG_FIFO_EN|\
        0<<YG_FIFO_EN|\
        0<<XG_FIFO_EN|\
        0<<TEMP_FIFO_EN,
    .intPinCfg =
        1<<I2C_BYPASS_EN|\
        0<<FSYNC_INT_MODE_EN|\
        0<<ACTL_FSYNC|\
        0<<INT_ANYRD_2CLEAR|\
        0<<LATCH_INT_EN|\
        0<<INT_OPEN|\
        0<<ACTL,
    .intPinEnable =
        1<<DATA_RDY_EN|\
        0<<I2C_MST_INT_EN|\
        0<<FIFO_OFLOW_EN,
    .gyroRangeSetting = GYRO_500DPS,
    .fChoiceBSetting = FCHOICE_B_DISABLED,
    .accelRangeSetting = ACCEL_2G,
    .accelFChoiceBSetting = ACCEL_FCHOICE_B_DISABLED,
    .accelDlpfSetting = ACCEL_DLPF_10p2Hz,
    .pwrMgmtSetting =
        INTERNAL_CLK<<CLKSEL|\
        0<<TEMP_DIS|\
        0<<CYCLE|\
        0<<SLEEP|\
        0<<DEVICE_RESET,
    .magControlSetting = COUNTINIOUS_MODE_2<<MAG_OUTPUT_MODE,
};

static const uint8_t accelRange[ACCEL_16G+1] = { 2, 4, 8, 16};
static const uint16_t gyroRange[GYRO_2000DPS+1] = { 250, 500, 1000, 2000 };
static const uint16_t magRange = AK8362_MAX_RANGE;

/**
 * @brief Read a accelCalAxis of bytes from a MPU9250 sensor registers
 */
static esp_err_t imu_register_read(uint8_t device_addr, uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, device_addr, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte to a MPU9250 sensor register
 */
static esp_err_t imu_register_write_byte(uint8_t device_addr, uint8_t reg_addr, uint8_t data)
{
    esp_err_t ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, device_addr, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

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

void imu_hal_init()
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, SMPLRT_DIV, imu_conf.sampleDivSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, CONFIG, (imu_conf.fifoMode<<FIFO_MODE_OFFSET)|(imu_conf.fSyncSetting<<FSYNC_OFFSET)|imu_conf.dlpfSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, FIFO_EN, imu_conf.fifoEnSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, INT_PIN_CFG, imu_conf.intPinCfg));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, INT_ENABLE, imu_conf.intPinEnable));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, GYRO_CONFIG, (imu_conf.gyroRangeSetting<<GYRO_FS_SEL_OFFSET)|imu_conf.fChoiceBSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, ACCEL_CONFIG, imu_conf.accelRangeSetting<<ACCEL_FS_SEL_OFFSET));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, ACCEL_CONFIG_2, imu_conf.accelDlpfSetting|(imu_conf.accelFChoiceBSetting<<ACCEL_FCHOICE_B_OFFSET)));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, PWR_MGMT_1, imu_conf.intPinEnable));
    ESP_ERROR_CHECK(imu_register_write_byte(AK8362_SENSOR_ADDR, AK8362_CONTROL_1, imu_conf.magControlSetting));
}

//UNUSED FUNC
// static void imu_deinit(void)
// {
//     ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
// }

// static void imu_reset(void)
// {
//     ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, PWR_MGMT_1, 1 << DEVICE_RESET));
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


static void mag_read(ImuData_t data)
{
    uint8_t buffer[6];
    uint8_t overflow = 0;
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_1, &overflow, 1));
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_MAG_DATA, buffer, 6));
    data[MAG][X_AXIS] = ((int16_t)buffer[MAG_XOUT_H_OFFSET] << 8) | buffer[MAG_XOUT_L_OFFSET];
    data[MAG][Y_AXIS] = ((int16_t)buffer[MAG_YOUT_H_OFFSET] << 8) | buffer[MAG_YOUT_L_OFFSET];
    data[MAG][Z_AXIS] = ((int16_t)buffer[MAG_ZOUT_H_OFFSET] << 8) | buffer[MAG_ZOUT_L_OFFSET];
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_2, &overflow, 1));
}

void imu_read(ImuData_t data)
{
    DataOffsets_t offset = ACCEL_XOUT_H_OFFSET;
    uint8_t bufferSize = GYRO_ZOUT_L_OFFSET+1;
    uint8_t buffer[bufferSize];
    ESP_ERROR_CHECK(imu_register_read(MPU9250_SENSOR_ADDR, ACCEL_XOUT_H, buffer, bufferSize));
    for(uint8_t sensor=ACCEL; sensor<=GYRO; sensor++){
        if(sensor == GYRO){
            offset = GYRO_XOUT_H_OFFSET;
        }
        for(uint8_t axis=X_AXIS; axis<NO_AXIS; axis++){
            data[sensor][axis] = ((int16_t)buffer[2*axis+offset] << 8) | buffer[2*axis+offset+1];
        }
    }
    mag_read(data);
    return;
}

uint8_t imu_get_accel_range()
{
    return accelRange[imu_conf.accelRangeSetting];
}

uint16_t imu_get_gyro_range()
{
    return gyroRange[imu_conf.gyroRangeSetting];
}

uint16_t imu_get_mag_range()
{
    return magRange;
}