#include "imu_helpers.h"
#include <string.h>

// SLOW!!
static float map_int16_to_range(int16_t value, int16_t range) {
  return ((float)value - INT16_MIN) * 2 * range / (INT16_MAX - INT16_MIN) -
         range;
}
// SLOW!!

// SLOW
static void convertRaw(ImuData_t raw, float data[NO_SENSOR][NO_AXIS]) {
  uint16_t range;
  for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
    switch (sensor) {
    case ACCEL:
      range = (int16_t)imu_get_accel_range();
      break;

    case GYRO:
      range = imu_get_gyro_range();
      break;

    case MAG:
      range = imu_get_mag_range();
      break;

    default:
      assert(0);
      break;
    }

    for (uint8_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      data[sensor][axis] = map_int16_to_range(raw[sensor][axis], range);
    }
  }
  return;
}
// SLOW

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

void preparePacket(ImuData_t data, Packet_t *packet) {
  float conveted_data[NO_SENSOR][NO_AXIS];
  char buffer[MAX_SINGLE_READING_SIZE];
  convertRaw(data, conveted_data);
  for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
    for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      sprintf(buffer, "%.2f,", conveted_data[sensor][axis]);
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

void accelUpdateBuffer(ImuData_t read, AccelCalibrationBuffer_t *buffer,
                       Axis_t axis, Direction_t direction) {
  buffer->sums[direction][axis] += read[ACCEL][axis];
  buffer->samples[direction][axis]++;
}

void accelCalculateBiasAndScale(AccelCalibrationBuffer_t *buffer,
                                AccelCalibrationData_t *data) {
  uint8_t range = imu_get_accel_range();
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

void accelApplyBiasAndScale(ImuData_t output, AccelCalibrationData_t *data) {
  Sensor_t sensor = ACCEL;
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    output[sensor][axis] = (int16_t)(data->scale[axis] * output[sensor][axis] /
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

void gyroUpdateBuffer(ImuData_t read, GyroCalibrationBuffer_t *buffer) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    buffer->sums[axis] += read[GYRO][axis];
  }
  buffer->samples++;
}

void gyroCalculateBias(GyroCalibrationBuffer_t *buffer,
                       GyroCalibrationData_t *data) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    data->bias[axis] = (int16_t)(buffer->sums[axis] / buffer->samples);
    ESP_LOGI("IMU-HAL", "axis: %d, bias:%d", axis, data->bias[axis]);
  }
  data->completed = true;
}

void gyroApplyBias(ImuData_t output, GyroCalibrationData_t *data) {
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    output[GYRO][axis] = output[GYRO][axis] - data->bias[axis];
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

void magApplyTransformMatrix(ImuData_t output, MagCalibrationData_t *data) {
  Matrix16_t transformMatrix;
  allocateMatrix16(3, 3, &transformMatrix);
  int16_t transformScalers[3][3] = {
      {367, -358, 826}, {848, -446, -570}, {-560, -890, -137}};
  for (uint8_t row = 0; row < 3; row++) {
    addRowMatrix16(transformScalers[row], row, &transformMatrix);
  }
  printMatrix16(data->transformMatrix);
  Matrix16_t magReadVector;
  Matrix32_t outputVector;
  allocateMatrix16(1, 3, &magReadVector);
  addRowMatrix16(output[MAG], 0, &magReadVector);
  multiplyMatrix16(&magReadVector, &transformMatrix, &outputVector);
  for (uint8_t row = 0; row < outputVector.numOfRows; row++) {
    output[MAG][row] = outputVector.m[row][0] / INVERSE_MATRIX_SCALER;
  }
  freeMatrix16(&transformMatrix);
  freeMatrix16(&magReadVector);
  freeMatrix32(&outputVector);
}
