#include "unity.h"
#include "imu_ao.h"

TEST_CASE("When gyro buffer is cleared", "gyroBufferClear unit test"){
    GyroCalibrationBuffer_t buffer;
    vector32_t testSums = { 0, 0, 0 };
    uint16_t testSamples = 0;
    gyroBufferClear(&buffer);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testSums, buffer.sums, NO_AXIS );
    TEST_ASSERT_EQUAL( testSamples, buffer.samples);
}

TEST_CASE("When gyro buffer is updated", "gyroUpdateBuffer unit test"){
    GyroCalibrationBuffer_t buffer;
    gyroBufferClear(&buffer);
    ImuData_t read = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };
    vector32_t testSums = { 2, 2, 2 };
    uint16_t testSamples = 2;
    gyroBufferClear(&buffer);
    gyroUpdateBuffer(read, &buffer);
    gyroUpdateBuffer(read, &buffer);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testSums, buffer.sums, NO_AXIS );
    TEST_ASSERT_EQUAL( testSamples, buffer.samples);
}

TEST_CASE("When gyro bias is calculated", "gyroCalculateBias unit test"){
    GyroCalibrationBuffer_t buffer;
    GyroCalibrationData_t data;
    gyroBufferClear(&buffer);
    ImuData_t read = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };
    vector16_t testBias = { 1, 1, 1 };
    gyroUpdateBuffer(read, &buffer);
    gyroUpdateBuffer(read, &buffer);
    gyroCalculateBias(&buffer, &data);
    TEST_ASSERT_EQUAL_INT16_ARRAY(testBias, data.bias, NO_AXIS);
}

TEST_CASE("When gyro bias is applied", "gyroApplyBias unit test"){
    GyroCalibrationData_t data = {
        .bias = { 1, 1, 1 },
        .completed = true
    };

    ImuData_t read = {
        {1, 1, 1},
        {10, 8, 5},
        {1, 1, 1}
    };
    vector16_t testBiasedRead = { 9, 7, 4 };
    gyroApplyBias(read, &data);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testBiasedRead, read[GYRO], NO_AXIS );
}
