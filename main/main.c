#include <stdio.h>
#include <imu-manager.h>

ImuConfig_t conf = {
    .sampleDivSetting = 0,
    .fSyncSetting = FSYNC_DISABLED,
    .dlpfSetting = DLPF_5Hz,
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
        0<<INT_LEVEL|\
        0<<INT_OPEN|\
        0<<LATCH_INT_EN|\
        0<<INT_RD_CLEAR|\
        0<<FSYNC_INT_LEVEL|\
        0<<FSYNC_INT_EN|\
        0<<I2C_BYPASS_EN,
    .intPinEnable =
        1<<DATA_RDY_EN|\
        0<<I2C_MST_INT_EN|\
        0<<FIFO_OFLOW_EN,
    .gyroRangeSetting = GYRO_250DPS,
    .accelRangeSetting = ACCEL_2G,
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
    imu_who_am_i();
    for (int i=0; i<10; i++)
    {
        imu_read();
    }
    imu_deinit();
}
