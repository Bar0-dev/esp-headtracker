#ifndef IMU_HELPERS_H
#define IMU_HELPERS_H

#include "imu_hal.h"
#include "packet.h"

#define MAX_MAG_CALIBRATION_SAMPLES 800 //equivalent of 8s of calibration

typedef char* AxisString_t;

typedef float outputVector_t[NO_AXIS];
typedef outputVector_t  outputMatrix_t [NO_SENSOR];

typedef struct
{
    vector16_t bias;
    vector16_t scale;
    bool completed;
} AccelCalibrationData_t;

typedef struct
{
    vector16_t bias;
    bool completed;
} GyroCalibrationData_t;

typedef struct
{
    vector16_t bias;
    vector16_t transformMatrix[NO_AXIS];
    bool completed;
} MagCalibrationData_t;

typedef struct 
{
    AccelCalibrationData_t accel;
    GyroCalibrationData_t gyro;
    MagCalibrationData_t mag;
} CalibtrationData_t;

typedef struct
{
    vector32_t sums[NO_DIRECTION];
    uint16_t samples;
} AccelCalibrationBuffer_t;

typedef struct
{
    vector32_t sums;
    uint16_t samples;
} GyroCalibrationBuffer_t;

typedef struct
{
    ImuData_t read;
    vector16_t stored[MAX_MAG_CALIBRATION_SAMPLES];
} MagCalibrationBuffe_t;

void calibrationSetNotCompleted(CalibtrationData_t * calData);
void getAxisName(Axis_t axis, AxisString_t axisString);
void preparePacket(ImuData_t data, packet_t * packet);
void accelBufferClear(AccelCalibrationBuffer_t * buffer);
void accelUpdateBuffer(ImuData_t read, AccelCalibrationBuffer_t * buffer, Axis_t axis, Direction_t direction);
void accelCalculateBiasAndScale(AccelCalibrationBuffer_t * buffer, AccelCalibrationData_t * data);
void accelApplyBiasAndScale(ImuData_t output, AccelCalibrationData_t * data);
void gyroBufferClear(GyroCalibrationBuffer_t * buffer);
void gyroUpdateBuffer(ImuData_t read, GyroCalibrationBuffer_t * buffer);
void gyroCalculateBias(GyroCalibrationBuffer_t * buffer, GyroCalibrationData_t * data);
void gyroApplyBias(ImuData_t output, GyroCalibrationData_t * data);

#endif