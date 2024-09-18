#ifndef IMU_HELPERS_H
#define IMU_HELPERS_H

#include "core.h"
#include "imu_hal.h"
#include <stdbool.h>

typedef char *AxisString_t;

typedef struct {
  Vector16_t bias;
  Vector16_t scale;
  bool completed;
} AccelCalibrationData_t;

typedef struct {
  Vector16_t bias;
  bool completed;
} GyroCalibrationData_t;

typedef struct {
  Matrix16_t *transformMatrix;
  bool completed;
} MagCalibrationData_t;

typedef struct {
  AccelCalibrationData_t accel;
  GyroCalibrationData_t gyro;
  MagCalibrationData_t mag;
} CalibtrationData_t;

typedef struct {
  Vector32_t sums[NO_DIRECTION];
  Vector16_t samples[NO_DIRECTION];
} AccelCalibrationBuffer_t;

typedef struct {
  Vector32_t sums;
  uint16_t samples;
} GyroCalibrationBuffer_t;

typedef struct {
  ImuData_t read;
  Vector16_t stored[MAX_MAG_CALIBRATION_SAMPLES];
} MagCalibrationBuffe_t;

void convertRaw(ImuData_t raw, float data[NO_SENSOR][NO_AXIS]);
void calibrationSetNotCompleted(CalibtrationData_t *calData);
void getAxisName(Axis_t axis, AxisString_t axisString);
void prepareRawPacket(ImuData_t data, Packet_t *packet);
void preparePacket(ImuData_t data, Packet_t *packet);
void accelBufferClear(AccelCalibrationBuffer_t *buffer);
void accelUpdateBuffer(Buffer_t *sensorBuffer, AccelCalibrationBuffer_t *buffer,
                       Axis_t axis, Direction_t direction);
void accelCalculateBiasAndScale(AccelCalibrationBuffer_t *buffer,
                                AccelCalibrationData_t *data);
void accelApplyBiasAndScale(ImuData_t output, AccelCalibrationData_t *data);
void gyroBufferClear(GyroCalibrationBuffer_t *buffer);
void gyroUpdateBuffer(Buffer_t *sensorBuffer, GyroCalibrationBuffer_t *buffer);
void gyroCalculateBias(GyroCalibrationBuffer_t *buffer,
                       GyroCalibrationData_t *data);
void gyroApplyBias(ImuData_t output, GyroCalibrationData_t *data);
void loadMagTransformationMatrix(MagCalibrationData_t *data);
void magApplyTransformMatrix(ImuData_t output, MagCalibrationData_t *data);

#endif
