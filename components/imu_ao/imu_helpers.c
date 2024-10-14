#include "imu_helpers.h"
#include "core.h"
#include "esp_log.h"
#include "imu_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static float map_int16_to_range(int16_t value, int16_t range) {
  return ((float)value - INT16_MIN) * 2 * range / (INT16_MAX - INT16_MIN) -
         range;
}

// TODO: change mag mapping to 32760
void convertRaw(ImuData_t *raw, FusionSensorData_t *out) {
  uint16_t rangeAccel = (int16_t)imu_hal_get_accel_range();
  uint16_t rangeGyro = imu_hal_get_gyro_range();
  uint16_t rangeMag = imu_hal_get_mag_range();

  for (uint8_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    out->sensor.accel.array[axis] =
        map_int16_to_range((*raw)[ACCEL][axis], rangeAccel);
    out->sensor.gyro.array[axis] =
        map_int16_to_range((*raw)[GYRO][axis], rangeGyro);
    out->sensor.mag.array[axis] =
        map_int16_to_range((*raw)[MAG][axis], rangeMag);
  }
}

AxisString_t axes[] = {
    "X_AXIS",
    "Y_AXIS",
    "Z_AXIS",
};

void getAxisName(Axis_t axis, AxisString_t axisString) {
  strcpy(axisString, axes[axis]);
}

void calibrationSetNotCompleted(CalibtrationData_t *calData) {
  calData->accel.completed = false;
  calData->gyro.completed = false;
  calData->mag.completed = false;
}

void prepareRawPacket(ImuData_t data, Packet_t *packet) {
  char buffer[MAX_SINGLE_READING_SIZE];
  // for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
  for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
    for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      sprintf(buffer, "%d,", data[sensor][axis]);
      packet->length += strlen(buffer);
      strcat(packet->payload, buffer);
    }
  }
  strcat(packet->payload, "\n");
  packet->length++;
}

void accelBufferClear(AccelCalibrationBuffer_t *buffer) {
  for (Direction_t direction = POSITIVE; direction < NO_DIRECTION;
       direction++) {
    for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      buffer->sums[direction][axis] = 0;
      buffer->samples[direction][axis] = 0;
    }
  }
}

void accelUpdateBuffer(Buffer_t *sensorBuffer, AccelCalibrationBuffer_t *buffer,
                       Axis_t axis, Direction_t direction) {
  for (uint8_t index = 0; index < sensorBuffer->length; index++) {
    buffer->sums[direction][axis] +=
        sensorBuffer->data[index].read[ACCEL][axis];
    buffer->samples[direction][axis]++;
  }
}

void accelCalculateBiasAndScale(AccelCalibrationBuffer_t *buffer,
                                AccelCalibrationData_t *data) {
  uint8_t range = imu_hal_get_accel_range();
  int16_t ymax = INT16_MAX / range;
  int16_t ymin = INT16_MIN / range;
  int16_t xmax;
  int16_t xmin;
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    // Guard to not divide by zero
    if ((buffer->samples[POSITIVE][axis] == 0) ||
        (buffer->samples[NEGATIVE][axis] == 0)) {
      return (void)0;
    }
    int16_t avga = (int16_t)(buffer->sums[POSITIVE][axis] /
                             buffer->samples[POSITIVE][axis]);
    int16_t avgb = (int16_t)(buffer->sums[NEGATIVE][axis] /
                             buffer->samples[NEGATIVE][axis]);
    xmax = (avga > avgb) ? avga : avgb;
    xmin = (avga < avgb) ? avga : avgb;
    data->scale[axis] =
        (int16_t)(ACCEL_SCALE_FACTOR * (ymax - ymin) / (xmax - xmin));
    data->bias[axis] = ymax - xmax * (ymax - ymin) / (xmax - xmin);
    ESP_LOGI("IMU-HAL", "axis: %d, scale:%d, bias:%d", axis, data->scale[axis],
             data->bias[axis]);
  }
  data->completed = true;
}

void accelApplyBiasAndScale(ImuData_t *output, AccelCalibrationData_t *data) {
  Sensor_t sensor = ACCEL;
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    (*output)[sensor][axis] =
        (int16_t)(data->scale[axis] * (*output)[sensor][axis] /
                  ACCEL_SCALE_FACTOR) +
        data->bias[axis];
  }
}

void gyroBufferClear(GyroCalibrationBuffer_t *buffer) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    buffer->sums[axis] = 0;
  }
  buffer->samples = 0;
}

void gyroUpdateBuffer(Buffer_t *sensorBuffer, GyroCalibrationBuffer_t *buffer) {
  for (uint8_t index = 0; index < sensorBuffer->length; index++) {
    for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      buffer->sums[axis] += sensorBuffer->data[index].read[GYRO][axis];
    }
    buffer->samples++;
  }
}

void gyroCalculateBias(GyroCalibrationBuffer_t *buffer,
                       GyroCalibrationData_t *data) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    data->bias[axis] = (int16_t)(buffer->sums[axis] / buffer->samples);
    ESP_LOGI("IMU-HAL", "axis: %d, bias:%d", axis, data->bias[axis]);
  }
  data->completed = true;
}

void gyroApplyBias(ImuData_t *output, GyroCalibrationData_t *data) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    (*output)[GYRO][axis] = (*output)[GYRO][axis] - data->bias[axis];
  }
}

void loadMagTransformationMatrix(MagCalibrationData_t *data) {
  Matrix16_t transformMatrix;
  allocateMatrix16(3, 3, &transformMatrix);
  int16_t transformScalers[3][3] = {
      {367, -358, 826}, {848, -446, -570}, {-560, -890, -137}};
  for (uint8_t row = 0; row < 3; row++) {
    addRowMatrix16(transformScalers[row], row, &transformMatrix);
  }
  data->transformMatrix = &transformMatrix;
  printMatrix16(data->transformMatrix);
  freeMatrix16(&transformMatrix);
}

// TODO: Change this to use calibration data
void magApplyTransformMatrix(ImuData_t *output) {
  Vector16_t biasVector = {-61, -70, -265};
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    (*output)[MAG][axis] = (*output)[MAG][axis] - biasVector[axis];
  }
  Matrix16_t transformMatrix;
  allocateMatrix16(3, 3, &transformMatrix);
  int16_t transformScalers[3][3] = {
      {-8781, 3990, 4265}, {4620, 8604, 1508}, {2782, -3565, 8481}};
  for (uint8_t row = 0; row < 3; row++) {
    addRowMatrix16(transformScalers[row], row, &transformMatrix);
  }
  Matrix16_t magReadVector;
  Matrix32_t outputVector;
  allocateMatrix16(1, 3, &magReadVector);
  addRowMatrix16((*output)[MAG], 0, &magReadVector);
  multiplyMatrix16(&magReadVector, &transformMatrix, &outputVector);
  for (uint8_t column = 0; column < outputVector.numOfCols; column++) {
    (*output)[MAG][column] = outputVector.m[0][column] / INVERSE_MATRIX_SCALER;
  }
  freeMatrix16(&transformMatrix);
  freeMatrix16(&magReadVector);
  freeMatrix32(&outputVector);
}
