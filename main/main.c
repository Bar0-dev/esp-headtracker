#include <stdio.h>
#include <imu-manager.h>

ImuConfig_t conf = {
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
};

void app_main(void)
{
    imu_init(conf);
    imu_who_am_i(AK8362_SENSOR_ADDR);
    while(1)
    {
        imu_read();
    }
    imu_deinit();
}
