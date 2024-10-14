#include "core.h"
#include "esp_log.h"
#include "imu_ao.h"
#include "test_helpers.h"
#include "unity.h"
#include <math.h>
#include <stdint.h>

TEST_CASE("When accel buffer is cleared", "accelBufferClear unit test") {
  AccelCalibrationBuffer_t buffer;
  Vector32_t testSums = {0, 0, 0};
  Vector16_t testSamples = {0, 0, 0};
  accelBufferClear(&buffer);
  for (Direction_t direction = POSITIVE; direction < NO_DIRECTION;
       direction++) {
    TEST_ASSERT_EQUAL_INT32_ARRAY(testSums, buffer.sums[direction], NO_AXIS);
    TEST_ASSERT_EQUAL_INT16_ARRAY(testSamples, buffer.samples[direction],
                                  NO_AXIS);
  }
}

TEST_CASE("When accel buffer is updated", "accelUpdateBuffer unit test") {
  uint8_t bufferReads = MAX_BUFFER_SIZE;
  AccelCalibrationBuffer_t calibrationBuffer;
  ImuData_t readPos = {{1, 2, 4}, {1, 1, 1}, {1, 1, 1}};
  ImuData_t readNeg = {{-1, -2, -4}, {1, 1, 1}, {1, 1, 1}};
  accelBufferClear(&calibrationBuffer);
  Buffer_t sensorBufferPos;
  Buffer_t sensorBufferNeg;
  set_buffer_instance(&sensorBufferPos, readPos);
  set_buffer_instance(&sensorBufferNeg, readNeg);
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    for (Direction_t direction = POSITIVE; direction < NO_DIRECTION;
         direction++) {
      if (direction == POSITIVE) {
        accelUpdateBuffer(&sensorBufferPos, &calibrationBuffer, axis,
                          direction);
      } else {
        accelUpdateBuffer(&sensorBufferNeg, &calibrationBuffer, axis,
                          direction);
      }
    }
  }
  Vector32_t testSumsPos = {bufferReads * 1, bufferReads * 2, bufferReads * 4};
  Vector32_t testSumsNeg = {bufferReads * (-1), bufferReads * (-2),
                            bufferReads * (-4)};
  Vector16_t testSamples = {bufferReads, bufferReads, bufferReads};
  for (Direction_t direction = POSITIVE; direction < NO_DIRECTION;
       direction++) {
    if (direction == POSITIVE) {
      TEST_ASSERT_EQUAL_INT32_ARRAY(testSumsPos,
                                    calibrationBuffer.sums[direction], NO_AXIS);
      TEST_ASSERT_EQUAL_INT16_ARRAY(
          testSamples, calibrationBuffer.samples[direction], NO_AXIS);
    } else {
      TEST_ASSERT_EQUAL_INT32_ARRAY(testSumsNeg,
                                    calibrationBuffer.sums[direction], NO_AXIS);
      TEST_ASSERT_EQUAL_INT16_ARRAY(
          testSamples, calibrationBuffer.samples[direction], NO_AXIS);
    }
  }
}

TEST_CASE("When accel bias and scale is calculated",
          "accelCalculateBiasAndScale unit test") {
  AccelCalibrationBuffer_t buffer;
  AccelCalibrationData_t data;
  accelBufferClear(&buffer);
  uint8_t range = imu_hal_get_accel_range(); //+-2G
  // int16_t oneGRead = INT16_MAX/range;
  // int16_t negOneGRead = INT16_MIN/range;
  int16_t readValpos = (INT16_MAX / range) * 0.95;
  int16_t readValneg = (INT16_MIN / range) * 0.9;
  // equation for the line that passed through {oneGRead, posRead},
  // {negOneGRead, negRead} is y=1.081*ACCEL_SCALE_FACTOR*x-442
  int16_t scale = (int16_t)(1.081 * ACCEL_SCALE_FACTOR);
  int16_t bias = -442;
  Vector16_t testScale = {scale, scale, scale};
  Vector16_t testBias = {bias, bias, bias};
  ImuData_t readPos = {
      {readValpos, readValpos, readValpos}, {1, 1, 1}, {1, 1, 1}};
  ImuData_t readNeg = {
      {readValneg, readValneg, readValneg}, {1, 1, 1}, {1, 1, 1}};
  Buffer_t sensorBufferPos;
  Buffer_t sensorBufferNeg;
  set_buffer_instance(&sensorBufferPos, readPos);
  set_buffer_instance(&sensorBufferNeg, readNeg);
  for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
    for (Direction_t direction = POSITIVE; direction < NO_DIRECTION;
         direction++) {
      if (direction == POSITIVE) {
        accelUpdateBuffer(&sensorBufferPos, &buffer, axis, direction);
      } else {
        accelUpdateBuffer(&sensorBufferNeg, &buffer, axis, direction);
      }
    }
  }
  ESP_LOGI("DEBUG", "%ld, %d", buffer.sums[POSITIVE][X_AXIS],
           buffer.samples[POSITIVE][X_AXIS]);
  ESP_LOGI("DEBUG", "%ld, %d", buffer.sums[NEGATIVE][X_AXIS],
           buffer.samples[NEGATIVE][X_AXIS]);
  accelCalculateBiasAndScale(&buffer, &data);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testBias, data.bias, NO_AXIS);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testScale, data.scale, NO_AXIS);
  TEST_ASSERT_EQUAL(data.completed, true);
}

TEST_CASE("When accel bias and scale is applied",
          "accelApplyBiasAndScale unit test") {
  int16_t scale = 1000;
  int16_t bias = -100;
  uint8_t range = 2; //+-2G
  AccelCalibrationData_t data = {.scale = {scale, scale, scale},
                                 .bias = {bias, bias, bias},
                                 .completed = true};

  int16_t posRead = (INT16_MAX / range) * 0.95;
  ImuData_t read = {{posRead, posRead, posRead}, {1, -1, 1}, {1, 1, 1}};
  int16_t expectedRead = scale / ACCEL_SCALE_FACTOR * posRead + bias;
  Vector16_t testScaledBiasedOutput = {expectedRead, expectedRead,
                                       expectedRead};
  accelApplyBiasAndScale(&read, &data);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testScaledBiasedOutput, read[ACCEL], NO_AXIS);
}

TEST_CASE("When gyro buffer is cleared", "gyroBufferClear unit test") {
  GyroCalibrationBuffer_t buffer;
  Vector32_t testSums = {0, 0, 0};
  uint16_t testSamples = 0;
  gyroBufferClear(&buffer);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testSums, buffer.sums, NO_AXIS);
  TEST_ASSERT_EQUAL(testSamples, buffer.samples);
}

TEST_CASE("When gyro buffer is updated", "gyroUpdateBuffer unit test") {
  uint8_t expected = MAX_BUFFER_SIZE;
  GyroCalibrationBuffer_t clibrationBuffer;
  Buffer_t dataBuffer;
  gyroBufferClear(&clibrationBuffer);
  ImuData_t read = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
  set_buffer_instance(&dataBuffer, read);
  Vector32_t testSums = {expected, expected, expected};
  uint16_t testSamples = expected;
  measure_execution_time2(gyroUpdateBuffer, &dataBuffer, &clibrationBuffer,
                          "gyroUpdateBuffer");
  TEST_ASSERT_EQUAL_INT16_ARRAY(testSums, clibrationBuffer.sums, NO_AXIS);
  TEST_ASSERT_EQUAL(testSamples, clibrationBuffer.samples);
}

TEST_CASE("When gyro bias is calculated", "gyroCalculateBias unit test") {
  uint8_t expected = 1;
  GyroCalibrationBuffer_t calibrationBuffer;
  GyroCalibrationData_t data;
  Buffer_t sensorBuffer;
  gyroBufferClear(&calibrationBuffer);
  ImuData_t read = {{1, 1, 1}, {1, 1, 1}, {1, 1, 1}};
  set_buffer_instance(&sensorBuffer, read);
  Vector16_t testBias = {expected, expected, expected};
  gyroUpdateBuffer(&sensorBuffer, &calibrationBuffer);
  gyroUpdateBuffer(&sensorBuffer, &calibrationBuffer);
  gyroCalculateBias(&calibrationBuffer, &data);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testBias, data.bias, NO_AXIS);
}

TEST_CASE("When gyro bias is applied", "gyroApplyBias unit test") {
  GyroCalibrationData_t data = {.bias = {1, 1, 1}, .completed = true};

  ImuData_t read = {{1, 1, 1}, {10, 8, 5}, {1, 1, 1}};
  Vector16_t testBiasedRead = {9, 7, 4};
  gyroApplyBias(read, &data);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testBiasedRead, read[GYRO], NO_AXIS);
}

TEST_CASE("When mag read transformed with matrix",
          "magApplyTransformMatrix unit test") {

  MagCalibrationData_t data;
  ImuData_t read = {{1, 1, 1}, {1, 1, 1}, {18, -14, 48}};
  Vector16_t testTransformed = {35, -47, 48};
  magApplyTransformMatrix(&read);
  TEST_ASSERT_EQUAL_INT16_ARRAY(testTransformed, read[MAG], NO_AXIS);
}

TEST_CASE("When mag raw read is converted", "convertRaw unit test") {
  uint8_t accelRange = imu_hal_get_accel_range();
  uint16_t gyroRange = imu_hal_get_gyro_range();
  uint16_t magRange = imu_hal_get_mag_range();

  int16_t accel1G = INT16_MAX / accelRange;
  int16_t gyro1DPS = INT16_MAX / gyroRange;
  int16_t mag1UT = 32760 / magRange;
  ImuData_t read = {{accel1G, -accel1G, accel1G},
                    {gyro1DPS, -gyro1DPS, gyro1DPS},
                    {mag1UT, -mag1UT, mag1UT}};
  float converted[NO_SENSOR][NO_AXIS];
  float expected[NO_SENSOR][NO_AXIS] = {
      {1.0, -1.0, 1.0},
      {1.0, -1.0, 1.0},
      {1.0, -1.0, 1.0},
  };
  convertRaw(read, converted);
  for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
    for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
      ESP_LOGI("Sensor, Axis, expected, converted", "%d, %d, %f, %f", sensor,
               axis, expected[sensor][axis], converted[sensor][axis]);
      TEST_ASSERT_EQUAL_FLOAT(expected[sensor][axis],
                              roundf(converted[sensor][axis]));
    }
  }
}
