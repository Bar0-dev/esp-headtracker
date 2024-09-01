#include "unity.h"
#include "core.h"

TEST_CASE("When matrix16_t is initialised", "allocateMatrix16 unit test"){
    Matrix16_t m1;
    uint16_t rows = 4;
    uint16_t columns = 3;
    allocateMatrix16(rows, columns, &m1);
    
    for(uint16_t row = 0; row < m1.numOfRows; row++)
    {
        for(uint16_t column = 0; column < m1.numOfCols; column++)
        {
            TEST_ASSERT_EQUAL_INT16( 0, m1.m[row][column] );
        }
    }
    freeMatrix16(&m1);
}

TEST_CASE("When adding row to matrix16_t", "addRowMatrix16 unit test"){
    Matrix16_t m1;
    uint16_t rows = 4;
    uint16_t columns = 4;
    allocateMatrix16(rows, columns, &m1);
    int16_t row[] = {1, 2, 3, 4};
    addRowMatrix16(row, 0, &m1);
    
    TEST_ASSERT_EQUAL_INT16_ARRAY( row, m1.m[0], m1.numOfCols);
    freeMatrix16(&m1);
}

TEST_CASE("When multiplying two matrix16_t", "multiplyMatrix16 unit test"){
    Matrix16_t m1;
    Matrix16_t m2;
    Matrix32_t mOut;
    Matrix32_t mExpected;
    uint16_t rows1 = 3;
    uint16_t columns1 = 4;
    uint16_t rows2 = 4;
    uint16_t columns2 = 3;
    allocateMatrix16(rows1, columns1, &m1);
    allocateMatrix16(rows2, columns2, &m2);
    allocateMatrix32(rows1, columns2, &mExpected);
    int16_t row1[] = {1, 2, 3, 4};
    int16_t row2[] = {1, 2, 3};
    int32_t rowExpected[] = {10, 20, 30};
    for(uint16_t row = 0; row < m1.numOfRows; row++){
        addRowMatrix16(row1, row, &m1);
    }
    for(uint16_t row = 0; row < m2.numOfRows; row++){
        addRowMatrix16(row2, row, &m2);
    }
    for(uint16_t row = 0; row < mExpected.numOfRows; row++){
        addRowMatrix32(rowExpected, row, &mExpected);
    }
    
    multiplyMatrix16(&m1, &m2, &mOut);
    
    for(uint16_t row = 0; row < mExpected.numOfRows; row++){
        TEST_ASSERT_EQUAL_INT32_ARRAY( mOut.m[row], mExpected.m[row] , mExpected.numOfCols);
    }
    freeMatrix16(&m1);
    freeMatrix16(&m2);
    freeMatrix32(&mOut);
    freeMatrix32(&mExpected);
}