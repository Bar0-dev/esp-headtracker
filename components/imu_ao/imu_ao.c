#include "include/imu_ao.h"

static const char *TAG = "imu_ao";

#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM              0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS       1000

static ImuConfig_t conf = {
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

/**
 * @brief Read a sequence of bytes from a MPU9250 sensor registers
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

static void imu_configure(ImuConfig_t config)
{
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, SMPLRT_DIV, config.sampleDivSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, CONFIG, (config.fifoMode<<FIFO_MODE_OFFSET)|(config.fSyncSetting<<FSYNC_OFFSET)|config.dlpfSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, FIFO_EN, config.fifoEnSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, INT_PIN_CFG, config.intPinCfg));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, INT_ENABLE, config.intPinEnable));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, GYRO_CONFIG, (config.gyroRangeSetting<<GYRO_FS_SEL_OFFSET)|config.fChoiceBSetting));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, ACCEL_CONFIG, config.accelRangeSetting<<ACCEL_FS_SEL_OFFSET));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, ACCEL_CONFIG_2, config.accelDlpfSetting|(config.accelFChoiceBSetting<<ACCEL_FCHOICE_B_OFFSET)));
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, PWR_MGMT_1, config.intPinEnable));
    ESP_ERROR_CHECK(imu_register_write_byte(AK8362_SENSOR_ADDR, AK8362_CONTROL_1, config.magControlSetting));
    ESP_LOGI(TAG, "IMU initialized successfully");
}
static void imu_deinit(void)
{
    ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    ESP_LOGI(TAG, "I2C de-initialized successfully");
}

static void imu_reset(void)
{
    ESP_ERROR_CHECK(imu_register_write_byte(MPU9250_SENSOR_ADDR, PWR_MGMT_1, 1 << DEVICE_RESET));
}

static uint8_t imu_who_am_i(uint8_t device_addr)
{
    uint8_t data;
    esp_err_t ret;
    if(device_addr == MPU9250_SENSOR_ADDR){
        ret = imu_register_read(device_addr, WHO_AM_I, &data, 1);
    } else {
        ret = imu_register_read(device_addr, AK8362_WHO_AM_I, &data, 1);
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "IMU who am I response: %X", data);
    return data;
}

static float map_int16_to_range(int16_t value, int16_t range)
{
    return ((float)value-INT16_MIN)*2*range/(INT16_MAX-INT16_MIN)-range;
}

static float convert_raw_accel_to_G(int16_t raw)
{
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

static float convert_raw_gyro_to_radPerS(int16_t raw)
{
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

static void mag_read(ImuData_t * data_raw)
{
    uint8_t buffer[6];
    uint8_t overflow = 0;
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_1, &overflow, 1));
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_MAG_DATA, buffer, 6));
    data_raw->mag.x = ((int16_t)buffer[MAG_XOUT_H_OFFSET] << 8) | buffer[MAG_XOUT_L_OFFSET];
    data_raw->mag.y = ((int16_t)buffer[MAG_YOUT_H_OFFSET] << 8) | buffer[MAG_YOUT_L_OFFSET];
    data_raw->mag.z = ((int16_t)buffer[MAG_ZOUT_H_OFFSET] << 8) | buffer[MAG_ZOUT_L_OFFSET];
    ESP_ERROR_CHECK(imu_register_read(AK8362_SENSOR_ADDR, AK8362_STATUS_2, &overflow, 1));
}

static void log_data(ImuData_t * data, SensorType_t sensor_type)
{
    Vector_t data_to_show;
    switch (sensor_type)
    {
    case ACCEL:
        data_to_show.x = convert_raw_accel_to_G(data->accel.x);
        data_to_show.y = convert_raw_accel_to_G(data->accel.y);
        data_to_show.z = convert_raw_accel_to_G(data->accel.z);
        break;

    case GYRO:
        data_to_show.x = convert_raw_gyro_to_radPerS(data->gyro.x);
        data_to_show.y = convert_raw_gyro_to_radPerS(data->gyro.y);
        data_to_show.z = convert_raw_gyro_to_radPerS(data->gyro.z);
        break;

    case MAG:
        data_to_show.x = map_int16_to_range(data->mag.x, AK8362_MAX_RANGE);
        data_to_show.y = map_int16_to_range(data->mag.y, AK8362_MAX_RANGE);
        data_to_show.z = map_int16_to_range(data->mag.z, AK8362_MAX_RANGE);
        break;

    default:
        break;
    }
    ESP_LOGI(TAG, "x, y, z:  %.2f   %.2f   %.2f", data_to_show.x,  data_to_show.y,  data_to_show.z);
}

ImuData_t imu_read_raw(void)
{
    uint8_t buffer[14];
    uint8_t interruptSignal = 0;
    while (interruptSignal == 0)
    {
        ESP_ERROR_CHECK(imu_register_read(MPU9250_SENSOR_ADDR, INT_STATUS, &interruptSignal, 1));
        // ESP_LOGI(TAG, "Interrupt signal: %u", interruptSignal);
    }
    ESP_ERROR_CHECK(imu_register_read(MPU9250_SENSOR_ADDR, ACCEL_XOUT_H, buffer, 14));
    ImuData_t data_raw = {
        .accel.x = ((int16_t)buffer[ACCEL_XOUT_H_OFFSET] << 8) | buffer[ACCEL_XOUT_L_OFFSET],
        .accel.y = ((int16_t)buffer[ACCEL_YOUT_H_OFFSET] << 8) | buffer[ACCEL_YOUT_L_OFFSET],
        .accel.z = ((int16_t)buffer[ACCEL_ZOUT_H_OFFSET] << 8) | buffer[ACCEL_ZOUT_L_OFFSET],
        .gyro.x = ((int16_t)buffer[GYRO_XOUT_H_OFFSET] << 8) | buffer[GYRO_XOUT_L_OFFSET],
        .gyro.y = ((int16_t)buffer[GYRO_YOUT_H_OFFSET] << 8) | buffer[GYRO_YOUT_L_OFFSET],
        .gyro.z = ((int16_t)buffer[GYRO_ZOUT_H_OFFSET] << 8) | buffer[GYRO_ZOUT_L_OFFSET],
    };
    mag_read(&data_raw);
    log_data(&data_raw, MAG);
    return data_raw;
}

/*
AO code
*/
//Forward declarations
State Imu_init(Imu * const me, Event const * const e);
State Imu_wait_for_button(Imu * const me, Event const * const e);
State Imu_position_calculation(Imu * const me, Event const * const e);
State Imu_position_calculation(Imu * const me, Event const * const e);

State Imu_init(Imu * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Imu_wait_for_button);
}

State Imu_wait_for_button(Imu * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_position_calculation);
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = IGNORED_STATUS;
        break;
    }
    return status;
}

State Imu_position_calculation(Imu * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        TimeEvent_arm(&me->positionCalculationLoopTimer);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_wait_for_button);
        break;

    case CALCULATE_POSITION_TIMER_SIG:
        imu_read_raw();
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        TimeEvent_disarm(&me->positionCalculationLoopTimer);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = IGNORED_STATUS;
        break;
    }
    return status;
}

void Imu_ctor(Imu * const me)
{
    Active_ctor(&me->super, (StateHandler)&Imu_init);
    TimeEvent_ctor(&me->positionCalculationLoopTimer, "IMU timer", (TickType_t)(POSITION_CALCULATION_PERIOD/portTICK_PERIOD_MS), pdTRUE, CALCULATE_POSITION_TIMER_SIG, &me->super);
    imu_configure(conf);
}
