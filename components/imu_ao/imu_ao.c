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

//SLOW!!
static float map_int16_to_range(int16_t value, int16_t range)
{
    return ((float)value-INT16_MIN)*2*range/(INT16_MAX-INT16_MIN)-range;
}
//SLOW!!

//SLOW!!
static void convert_raw_accel_to_G(RawVector_t const * const rawVector, Vector_t * const vector)
{
    AccelRangeConf_t range = conf.accelRangeSetting;
    uint8_t scale;

    switch (range)
    {
    case ACCEL_2G:
        scale = 2;
        break;
    case ACCEL_4G:
        scale = 4;
        break;
    case ACCEL_8G:
        scale = 8;
        break;
    case ACCEL_16G:
        scale = 16;
        break;
    default:
        break;
    }
    vector->x = map_int16_to_range(rawVector->x, scale);
    vector->y = map_int16_to_range(rawVector->y, scale);
    vector->z = map_int16_to_range(rawVector->z, scale);
    return;
}
//SLOW

//SLOW
static void convert_raw_gyro_to_radPerS(RawVector_t const * const rawVector, Vector_t * const vector)
{
    AccelRangeConf_t range = conf.accelRangeSetting;
    uint16_t scale;

    switch (range)
    {
    case GYRO_250DPS:
        scale = 250;
        break;
    case GYRO_500DPS:
        scale = 500;
        break;
    case GYRO_1000DPS:
        scale = 1000;
        break;
    case GYRO_2000DPS:
        scale = 2000;
        break;
    default:
        break;
    }
    vector->x = map_int16_to_range(rawVector->x, scale);
    vector->y = map_int16_to_range(rawVector->y, scale);
    vector->z = map_int16_to_range(rawVector->z, scale);
    return;
}
//SLOW

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
        convert_raw_accel_to_G(&data->accel, &data_to_show);
        break;

    case GYRO:
        convert_raw_gyro_to_radPerS(&data->gyro, &data_to_show);
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

static void imu_read_raw(ImuData_t * data_raw)
{
    uint8_t buffer[14];
    // uint8_t interruptSignal = 0;
    // while (interruptSignal == 0)
    // {
    //     ESP_ERROR_CHECK(imu_register_read(MPU9250_SENSOR_ADDR, INT_STATUS, &interruptSignal, 1));
    //     // ESP_LOGI(TAG, "Interrupt signal: %u", interruptSignal);
    // }
    ESP_ERROR_CHECK(imu_register_read(MPU9250_SENSOR_ADDR, ACCEL_XOUT_H, buffer, 14));
    data_raw->accel.x = ((int16_t)buffer[ACCEL_XOUT_H_OFFSET] << 8) | buffer[ACCEL_XOUT_L_OFFSET],
    data_raw->accel.y = ((int16_t)buffer[ACCEL_YOUT_H_OFFSET] << 8) | buffer[ACCEL_YOUT_L_OFFSET],
    data_raw->accel.z = ((int16_t)buffer[ACCEL_ZOUT_H_OFFSET] << 8) | buffer[ACCEL_ZOUT_L_OFFSET],
    data_raw->gyro.x = ((int16_t)buffer[GYRO_XOUT_H_OFFSET] << 8) | buffer[GYRO_XOUT_L_OFFSET],
    data_raw->gyro.y = ((int16_t)buffer[GYRO_YOUT_H_OFFSET] << 8) | buffer[GYRO_YOUT_L_OFFSET],
    data_raw->gyro.z = ((int16_t)buffer[GYRO_ZOUT_H_OFFSET] << 8) | buffer[GYRO_ZOUT_L_OFFSET],
    mag_read(data_raw);
    // log_data(&data_raw, MAG);
    return;
}

static void addSample(RawVector_t * const rawVector, BufferVector_t * const buffer, Axis_t axis)
{
    switch (axis)
    {
    case X_AXIS:
        buffer->x += rawVector->x;
        break;

    case Y_AXIS:
        buffer->y += rawVector->y;
        break;
        
    case Z_AXIS:
        buffer->z += rawVector->z;
        break;
    
    default:
        buffer->x += rawVector->x;
        buffer->y += rawVector->y;
        buffer->z += rawVector->z;
        break;
    }
}

static void updateBuffer(ImuData_t * const data_raw, BufferVector_t * const buffer, SensorType_t sensor, Axis_t axis)
{
    switch (sensor)
    {
    case ACCEL:
        addSample(&data_raw->accel, buffer, axis);
        break;
    
    case GYRO:
        addSample(&data_raw->gyro, buffer, axis);
        break;

    case MAG:
        addSample(&data_raw->mag, buffer, axis);
        break;
    
    default:
        break;
    }
}

static void calculateAxisOffset(BufferVector_t const * const buffer, RawVector_t * const sensorOffset, uint16_t samples, Axis_t axis)
{
    switch (axis)
    {
    case X_AXIS:
        sensorOffset->x = buffer->x/samples;
        break;
    
    case Y_AXIS:
        sensorOffset->y = buffer->y/samples;
        break;
    
    case Z_AXIS:
        sensorOffset->z = buffer->z/samples;
        break;
    
    case NO_AXIS:
        sensorOffset->x = buffer->x/samples;
        sensorOffset->y = buffer->y/samples;
        sensorOffset->z = buffer->z/samples;
        break;
    
    default:
        break;
    }
}

static void calculateOffsets(BufferVector_t const * const buffer, ImuData_t * const offsets, uint16_t samples, SensorType_t sensor, Axis_t axis)
{
    switch (sensor)
    {
    case ACCEL:
        calculateAxisOffset(buffer, &offsets->accel, samples, axis);
        break;
    
    case GYRO:
        calculateAxisOffset(buffer, &offsets->gyro, samples, axis);
        break;
    
    case MAG:
        calculateAxisOffset(buffer, &offsets->mag, samples, axis);
        break;
    
    default:
        break;
    }
}

static void applyOffsets(ImuData_t * const data, ImuData_t const * const offsets)
{
    data->accel.x -= offsets->accel.x;
    data->accel.y -= offsets->accel.y;
    data->accel.z -= offsets->accel.z;
    data->gyro.x -= offsets->gyro.x;
    data->gyro.y -= offsets->gyro.y;
    data->gyro.z -= offsets->gyro.z;
    data->mag.x -= offsets->mag.x;
    data->mag.y -= offsets->mag.y;
    data->mag.z -= offsets->mag.z;
}

/*
AO code
*/
//Forward declarations

static void resetBufferAndSamples(Imu * const me)
{
    me->buffer.x = 0;
    me->buffer.y = 0;
    me->buffer.z = 0;
    me->samples = 0;
}

State Imu_init(Imu * const me, Event const * const e);
State Imu_top(Imu * const me, Event const * const e);
State Imu_idle(Imu * const me, Event const * const e);
State Imu_read(Imu * const me, Event const * const e);
State Imu_calibration(Imu * const me, Event const * const e);
State Imu_cal_accel(Imu * const me, Event const * const e);
State Imu_cal_gyro(Imu * const me, Event const * const e);
// State Imu_cal_mag(Imu * const me, Event const * const e);

State Imu_init(Imu * const me, Event const * const e)
{
    return transition(&me->super.super, (StateHandler)&Imu_idle);
}

State Imu_top(Imu * const me, Event const * const e)
{
    State status;
    switch (e->sig)
    {
    case ENTRY_SIG:
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_read);
        break;

    case EV_BUTTON_HOLD:
        status = transition(&me->super.super, (StateHandler)&Imu_calibration);
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Hsm_top);
        break;
    }
    return status;
}

State Imu_idle(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_IDLE;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_read(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_READING;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case IMU_READ_TIMEOUT_SIG:
        imu_read_raw(&me->data);
        applyOffsets(&me->data, &me->calibrationOffsets);
        // log_data(&me->data, ACCEL);
        log_data(&me->calibrationOffsets, ACCEL);
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        TimeEvent_disarm(&me->readTimer);
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_calibration(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    switch (e->sig)
    {
    case ENTRY_SIG:
        evt.sig = EV_IMU_CALIBRATION_READY;
        Active_post(AO_Broker, &evt);
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        status = transition(&me->super.super, (StateHandler)&Imu_cal_accel);
        break;

    case EV_BUTTON_DOUBLE_PRESS:
        status = transition(&me->super.super, (StateHandler)&Imu_idle);
        break;

    case IMU_AXIS_CAL_DONE:
        evt.sig = EV_IMU_CALIBRATION_DONE;
        Active_post(AO_Broker, &evt);
        me->calibrationSamplingInProgress = false;
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_top);
        break;
    }
    return status;
}

State Imu_cal_accel(Imu * const me, Event const * const e)
{
    State status;
    Event evt = { LAST_EVENT_FLAG, (void *)0 };
    ImuData_t data;

    switch (e->sig)
    {
    case ENTRY_SIG:
        resetBufferAndSamples(me);
        me->calibrationAxis = X_AXIS;
        me->calibrationSensor = ACCEL;
        TimeEvent_arm(&me->preCalibrationTimer);
        status = HANDLED_STATUS;
        break;

    case IMU_PRE_CALIBRATION_TIMEOUT_SIG:
        evt.sig = EV_IMU_CALIBRATION_IN_PROGRESS;
        Active_post(AO_Broker, &evt);
        TimeEvent_arm(&me->readTimer);
        TimeEvent_arm(&me->calibrationTimer);
        me->calibrationSamplingInProgress = true;
        status = HANDLED_STATUS;
        break; 

    case IMU_READ_TIMEOUT_SIG:
        imu_read_raw(&data);
        updateBuffer(&data, &me->buffer, me->calibrationSensor, me->calibrationAxis);
        me->samples++;
        status = HANDLED_STATUS;
        break;

    case IMU_CALIBRATION_TIMEOUT_SIG:
        TimeEvent_disarm(&me->readTimer);
        evt.sig = IMU_AXIS_CAL_DONE;
        Active_post(&me->super, &evt);
        if(me->calibrationAxis != Z_AXIS){
            me->calibrationAxis++;
        } else {
            me->calibrationAxis = NO_AXIS;
        }
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_PRESSED:
        if(!me->calibrationSamplingInProgress && me->calibrationAxis != NO_AXIS){
            TimeEvent_arm(&me->preCalibrationTimer);
            resetBufferAndSamples(me);
        }
        status = HANDLED_STATUS;
        break;

    case EV_BUTTON_HOLD:
        if(!me->calibrationSamplingInProgress){
            status = transition(&me->super.super, (StateHandler)&Imu_idle);
        }
        status = HANDLED_STATUS;
        break;

    case EXIT_SIG:
        calculateOffsets(&me->buffer, &me->calibrationOffsets, me->samples, me->calibrationSensor, me->calibrationAxis);
        me->calibrationSensor = NO_SENSOR;
        status = HANDLED_STATUS;
        break;
    
    default:
        status = super(&me->super.super, (StateHandler)&Imu_calibration);
        break;
    }
    return status;
}
void Imu_ctor(Imu * const me)
{
    Active_ctor(&me->super, (StateHandler)&Imu_init);
    TimeEvent_ctor(&me->readTimer, "IMU read timer", (TickType_t)(READ_PERIOD/portTICK_PERIOD_MS), pdTRUE, IMU_READ_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->calibrationTimer, "IMU calibration timer", (TickType_t)(ACCEL_GYRO_CALIBRATION_PERIOD/portTICK_PERIOD_MS), pdFALSE, IMU_CALIBRATION_TIMEOUT_SIG, &me->super);
    TimeEvent_ctor(&me->preCalibrationTimer, "IMU pre calibration timer", (TickType_t)(PRE_CALIBRATION_PERIOD/portTICK_PERIOD_MS), pdFALSE, IMU_PRE_CALIBRATION_TIMEOUT_SIG, &me->super);
    imu_configure(conf);

    me->calibrationSamplingInProgress = false;
    me->calibrationAxis = NO_AXIS;
    me->calibrationSensor = NO_SENSOR;
    resetBufferAndSamples(me);
}