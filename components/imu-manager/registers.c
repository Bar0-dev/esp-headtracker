
#define MPU6050_SENSOR_ADDR                 0x68        /*!< Slave address of the MPU6050 sensor */
#define MPU6050_WHO_AM_I_REG_ADDR           0x75        /*!< Register addresses of the "who am I" register */

#define MPU6050_SAMPLE_RATE_DIV_ADDR        0x19
typedef uint8_t SampleDiv_t;

#define MPU6050_FSYNC_DLPF_CONFIG_ADDR      0x1A
#define MPU6050_FSYNC_OFFSET                3
typedef enum
{
    FSYNC_DISABLED,
    FSYNC_TEMP_OUT,
    FSYNC_GYRO_XOUT,
    FSYNC_GYRO_YOUT,
    FSYNC_GYRO_ZOUT,
    FSYNC_ACCEL_XOUT,
    FSYNC_ACCEL_YOUT,
    FSYNC_ACCEL_ZOUT,
} FSyncConf_t;
typedef enum
{
    DLPF_260Hz,
    DLPF_184Hz,
    DLPF_94Hz,
    DLPF_44Hz,
    DLPF_21Hz,
    DLPF_10Hz,
    DLPF_5Hz,
} DlpfConf_t;

/**
 * @todo add slef test offset and enums for GYRO_CONFIG
*/

#define MPU6050_GYRO_CONFIG_ADDR       0x1B
#define MPU6050_GYRO_FS_SEL_OFFSET     3
typedef enum
{
    GYRO_250DPS,
    GYRO_500DPS,
    GYRO_1000DPS,
    GYRO_2000DPS,
} GyroConf_t;

/**
 * @todo add slef test offset and enums for ACCEL_CONFIG
*/

#define MPU6050_ACCEL_CONFIG_ADDR       0x1C
#define MPU6050_ACCEL_FS_SEL_OFFSET     3
typedef enum
{
    ACCEL_2G,
    ACCEL_4G,
    ACCEL_8G,
    ACCEL_16G,
} AccelConf_t;

#define MPU6050_FIFO_EN_ADDR         0x23
typedef uint8_t FifoEn_t;
typedef enum
{
SLV0_FIFO_EN,
SLV1_FIFO_EN,
SLV2_FIFO_EN,
ACCEL_FIFO_EN,
ZG_FIFO_EN,
YG_FIFO_EN,
XG_FIFO_EN,
TEMP_FIFO_EN,
} FifoEnOffsets_t;

#define MPU6050_INT_PIN_CFG_ADDR         0x37
typedef uint8_t IntPinCfg_t;
typedef enum
{
    I2C_BYPASS_EN = 1,
    FSYNC_INT_EN,
    FSYNC_INT_LEVEL,
    INT_RD_CLEAR,
    LATCH_INT_EN,
    INT_OPEN,
    INT_LEVEL,
} IntPinCfgOffsets_t;

#define MPU6050_INT_ENABLE_ADDR         0x38
typedef uint8_t IntPinEnable_t;
typedef enum
{
DATA_RDY_EN,
I2C_MST_INT_EN = 3,
FIFO_OFLOW_EN,
} IntEnable_t;


#define MPU6050_INT_STATUS_ADDR         0x3A
typedef enum
{
    DATA_RDY_INT,
    I2C_MST_INT = 3,
    FIFO_OFLOW_INT = 4,
} InterruptStatus_t;

#define MPU6050_ACCEL_DATA_ADDR         0x3B

#define MPU6050_TEMP_DATA_ADDR         0x41

#define MPU6050_GYRO_DATA_ADDR         0x43
typedef enum
{
    ACCEL_X_OUT_H,
    ACCEL_X_OUT_L,
    ACCEL_Y_OUT_H,
    ACCEL_Y_OUT_L,
    ACCEL_Z_OUT_H,
    ACCEL_Z_OUT_L,
    TEMP_OUT_H,
    TEMP_OUT_L,
    GYRO_X_OUT_H,
    GYRO_X_OUT_L,
    GYRO_Y_OUT_H,
    GYRO_Y_OUT_L,
    GYRO_Z_OUT_H,
    GYRO_Z_OUT_L,
} DataOffsets_t;

#define MPU6050_PWR_MGMT_1_ADDR         0x6B
typedef uint8_t PwrMgmt_t;
typedef enum
{
    CLKSEL,
    TEMP_DIS = 3,
    CYCLE = 5,
    SLEEP,
    DEVICE_RESET,
} PwrMgmtOffsets_t;
typedef enum
{
    INTERNAL_CLK,
    PPL_X_CLK,
    PPL_Y_CLK,
    PPL_Z_CLK,
    PPL_EX_32kHz,
    PPL_EX_19MHz,
    RESET_CLK = 7,
} ClkSrc_t;

#define MPU6050_FIFO_COUNT          0x72

#define MPU6050_FIFO_R_W            0x74

