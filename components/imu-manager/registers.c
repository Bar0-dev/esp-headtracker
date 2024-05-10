#define MPU9250_SENSOR_ADDR 0x68        /*!< Slave address of the MPU9250 sensor */

#define SELF_TEST_X_GYRO 0x0
#define SELF_TEST_Y_GYRO 0x1
#define SELF_TEST_Z_GYRO 0x2
#define SELF_TEST_X_ACCEL 0x0D
#define SELF_TEST_Y_ACCEL 0x0E
#define SELF_TEST_Z_ACCEL 0x0F
#define XG_OFFSET_H 0x13
#define XG_OFFSET_L 0x14
#define YG_OFFSET_H 0x15
#define YG_OFFSET_L 0x16
#define ZG_OFFSET_H 0x17
#define ZG_OFFSET_L 0x18
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define ACCEL_CONFIG_2 0x1D
#define LP_ACCEL_ODR 0x1E
#define WOM_THR 0x1F
#define FIFO_EN 0x23
#define I2C_MST_CTRL 0x24
#define I2C_SLVO_ADDR 0x25
#define I2C_SLVO_REG 0x26
#define I2C_SLVO_CTRL 0x27
#define I2C_SLV1_ADDR 0x28
#define I2C_SLV1_REG 0x29
#define I2C_SLV1_CTRL 0x2A
#define I2C_SLV2_ADDR 0x2B
#define I2C_SLV2_REG 0x2C
#define I2C_SLV2_CTRL 0x2D
#define I2C_SLV3_ADDR 0x2E
#define I2C_SLV3_REG 0x2F
#define I2C_SLV3_CTRL 0x30
#define I2C_SLV4_ADDR 0x31
#define I2C_SLV4_REG 0x32
#define I2C_SLV4_DO 0x33
#define I2C_SLV4_CTRL 0x34
#define I2C_SLV4_DI 0x35
#define I2C_MST_STATUS 0x36
#define INT_PIN_CFG 0x37
#define INT_ENABLE 0x38
#define INT_STATUS 0x3A
#define ACCEL_XOUT_H 0x3B
#define ACCEL_XOUT_L 0x3C
#define ACCEL_YOUT_H 0x3D
#define ACCEL_YOUT_L 0x3E
#define ACCEL_ZOUT_H 0x3F
#define ACCEL_ZOUT_L 0x40
#define TEMP_OUT_H 0x41
#define TEMP_OUT_L 0x42
#define GYRO_XOUT_H 0x43
#define GYRO_XOUT_L 0x44
#define GYRO_YOUT_H 0x45
#define GYRO_YOUT_L 0x46
#define GYRO_ZOUT_H 0x47
#define GYRO_ZOUT_L 0x48
#define EXT_SENS_DATA_00 0x49
#define EXT_SENS_DATA_01 0x4A
#define EXT_SENS_DATA_02 0x4B
#define EXT_SENS_DATA_03 0x4C
#define EXT_SENS_DATA_04 0x4D
#define EXT_SENS_DATA_05 0x4E
#define EXT_SENS_DATA_06 0x4F
#define EXT_SENS_DATA_07 0x50
#define EXT_SENS_DATA_08 0x51
#define EXT_SENS_DATA_09 0x52
#define EXT_SENS_DATA_10 0x53
#define EXT_SENS_DATA_11 0x54
#define EXT_SENS_DATA_12 0x55
#define EXT_SENS_DATA_13 0x56
#define EXT_SENS_DATA_14 0x57
#define EXT_SENS_DATA_15 0x58
#define EXT_SENS_DATA_16 0x59
#define EXT_SENS_DATA_17 0x5A
#define EXT_SENS_DATA_18 0x5B
#define EXT_SENS_DATA_19 0x5C
#define EXT_SENS_DATA_20 0x5D
#define EXT_SENS_DATA_21 0x5E
#define EXT_SENS_DATA_22 0x5F
#define EXT_SENS_DATA_23 0x60
#define I2C_SLVO_DO 0x63
#define I2C_SLV1_DO 0x64
#define I2C_SLV2_DO 0x65
#define I2C_SLV3_DO 0x66
#define I2C_MST_DELAY_CTRL 0x67
#define SIGNAL_PATH_RESET 0x68
#define MOT_DETECT_CTRL 0x69
#define USER_CTRL 0x6A
#define PWR_MGMT_1 0x6B
#define PWR_MGMT_2 0x6C
#define FIFO_COUNTH 0x72
#define FIFO_COUNTL 0x73
#define FIFO_R_W 0x74
#define WHO_AM_I 0x75
#define XA_OFFSET H 0x77
#define XA_OFFSET_L 0x78
#define YA_OFFSET_H 0x7A
#define YA_OFFSET_L 0x7B
#define ZA_OFFSET_H 0x7D
#define ZA_OFFSET_L 0x7E

typedef uint8_t SampleDiv_t;

#define FSYNC_OFFSET                3
#define FIFO_MODE_OFFSET 6
typedef enum 
{
    NO_OVERFLOW,
    ALLOW_OVERFLOW
} FifoModeConf_t;
typedef enum
{
    DLPF_250Hz,
    DLPF_184Hz,
    DLPF_92Hz,
    DLPF_41Hz,
    DLPF_20Hz,
    DLPF_10Hz,
    DLPF_5Hz,
    DLPF_3600Hz,
} DlpfConf_t;
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

/**
 * @todo add slef test offset and enums for GYRO_CONFIG
*/

#define GYRO_FS_SEL_OFFSET     3
typedef enum
{
    GYRO_250DPS,
    GYRO_500DPS,
    GYRO_1000DPS,
    GYRO_2000DPS,
} GyroRangeConf_t;

typedef enum
{
    FCHOICE_B_DISABLED,
    FCHOICE_B_8800Hz,
    FCHOICE_B_3600Hz,
} FChoiceBConf_t;
	
/**
 * @todo add slef test offset and enums for ACCEL_CONFIG
*/

#define ACCEL_FS_SEL_OFFSET     3
typedef enum
{
    ACCEL_2G,
    ACCEL_4G,
    ACCEL_8G,
    ACCEL_16G,
} AccelRangeConf_t;

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

typedef uint8_t IntPinEnable_t;
typedef enum
{
DATA_RDY_EN,
I2C_MST_INT_EN = 3,
FIFO_OFLOW_EN,
} IntEnable_t;


typedef enum
{
    DATA_RDY_INT,
    I2C_MST_INT = 3,
    FIFO_OFLOW_INT = 4,
} InterruptStatus_t;

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

typedef enum
{
    ACCEL_XOUT_H_OFFSET,
    ACCEL_XOUT_L_OFFSET,
    ACCEL_YOUT_H_OFFSET,
    ACCEL_YOUT_L_OFFSET,
    ACCEL_ZOUT_H_OFFSET,
    ACCEL_ZOUT_L_OFFSET,
    TEMP_OUT_H_OFFSET,
    TEMP_OUT_L_OFFSET,
    GYRO_XOUT_H_OFFSET,
    GYRO_XOUT_L_OFFSET,
    GYRO_YOUT_H_OFFSET,
    GYRO_YOUT_L_OFFSET,
    GYRO_ZOUT_H_OFFSET,
    GYRO_ZOUT_L_OFFSET,
} DataOffsets_t;