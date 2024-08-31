#include "unity.h"
#include "imu_ao.h"
#include "core.h"

TEST_CASE("When accel buffer is cleared", "accelBufferClear unit test"){
    AccelCalibrationBuffer_t buffer;
    Vector32_t testSums = { 0, 0, 0 };
    Vector16_t testSamples = { 0, 0, 0 };
    accelBufferClear(&buffer);
    for(Direction_t direction = POSITIVE; direction < NO_DIRECTION; direction++)
    {
        TEST_ASSERT_EQUAL_INT32_ARRAY( testSums, buffer.sums[direction], NO_AXIS );
        TEST_ASSERT_EQUAL_INT16_ARRAY( testSamples, buffer.samples[direction], NO_AXIS );
    }
}

TEST_CASE("When accel buffer is updated", "accelUpdateBuffer unit test"){
    uint8_t iterations = 10;
    AccelCalibrationBuffer_t buffer;
    ImuData_t read = {
        {1, 1, 1},
        {1, 1, 1},
        {1, 1, 1}
    };
    accelBufferClear(&buffer);
    for(uint8_t i = 0; i < iterations; i++){
        for(Direction_t direction = POSITIVE; direction < NO_DIRECTION; direction++)
        {
            for(Axis_t axis = X_AXIS; axis < NO_AXIS; axis++)
            {
                if(direction == NEGATIVE)
                {
                    read[ACCEL][axis] = -read[ACCEL][axis];
                }
                accelUpdateBuffer(read, &buffer, axis, direction);
            }
        }
    }
    Vector32_t testSums = { 0, 0, 0 };
    Vector16_t testSamples = { iterations, iterations, iterations };
    for(Direction_t direction = POSITIVE; direction < NO_DIRECTION; direction++)
    {
        TEST_ASSERT_EQUAL_INT32_ARRAY( testSums, buffer.sums[direction], NO_AXIS );
        TEST_ASSERT_EQUAL_INT16_ARRAY( testSamples, buffer.samples[direction], NO_AXIS );
    }
}

TEST_CASE("When accel bias and scale is calculated", "accelCalculateBiasAndScale unit test"){
    AccelCalibrationBuffer_t buffer;
    AccelCalibrationData_t data;
    accelBufferClear(&buffer);
    uint8_t range = 2; //+-2G
    // int16_t oneGRead = INT16_MAX/range;
    // int16_t negOneGRead = INT16_MIN/range;
    int16_t posRead = (INT16_MAX/range)*0.95;
    int16_t negRead = (INT16_MIN/range)*0.9;
    //equation for the line that passed through {oneGRead, posRead}, {negOneGRead, negRead} is y=1.081*ACCEL_SCALE_FACTOR*x-442
    int16_t scale = (int16_t)(1.081*ACCEL_SCALE_FACTOR);
    int16_t bias = -442;
    Vector16_t testScale = { scale, scale, scale };
    Vector16_t testBias = { bias, bias, bias };
    ImuData_t readPos = {
        {posRead, posRead, posRead},
        {1, 1, 1},
        {1, 1, 1}
    };
    ImuData_t readNeg = {
        {negRead, negRead, negRead},
        {1, 1, 1},
        {1, 1, 1}
    };
    for(Direction_t direction = POSITIVE; direction < NO_DIRECTION; direction++)
    {
        for(Axis_t axis = X_AXIS; axis < NO_AXIS; axis++)
        {
            if(direction == NEGATIVE)
            {
                accelUpdateBuffer(readNeg, &buffer, axis, direction);
            } else {

                accelUpdateBuffer(readPos, &buffer, axis, direction);
            }
            
        }
    }
    accelCalculateBiasAndScale(&buffer, &data);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testBias, data.bias, NO_AXIS );
    TEST_ASSERT_EQUAL_INT16_ARRAY( testScale, data.scale, NO_AXIS );
    TEST_ASSERT_EQUAL( data.completed, true);
}

TEST_CASE("When accel bias and scale is applied", "accelApplyBiasAndScale unit test"){
    int16_t scale = 1000;
    int16_t bias = -100;
    uint8_t range = 2; //+-2G
    AccelCalibrationData_t data = {
        .scale = { scale, scale, scale },
        .bias = { bias, bias, bias },
        .completed = true
    };

    int16_t posRead = (INT16_MAX/range)*0.95;
    ImuData_t read = {
        {posRead, posRead, posRead},
        {1, -1, 1},
        {1, 1, 1}
    };
    int16_t expectedRead = scale/ACCEL_SCALE_FACTOR*posRead+bias;
    Vector16_t testScaledBiasedOutput = { expectedRead, expectedRead, expectedRead };
    accelApplyBiasAndScale(read, &data);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testScaledBiasedOutput , read[ACCEL], NO_AXIS );
}

TEST_CASE("When gyro buffer is cleared", "gyroBufferClear unit test"){
    GyroCalibrationBuffer_t buffer;
    Vector32_t testSums = { 0, 0, 0 };
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
    Vector32_t testSums = { 2, 2, 2 };
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
    Vector16_t testBias = { 1, 1, 1 };
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
    Vector16_t testBiasedRead = { 9, 7, 4 };
    gyroApplyBias(read, &data);
    TEST_ASSERT_EQUAL_INT16_ARRAY( testBiasedRead, read[GYRO], NO_AXIS );
}
