#include "core.h"
#include "esp_log.h"

const static char * TAG = "CORE";

void allocateMatrix16(uint16_t rows, uint16_t columns, Matrix16_t * matrix)
{
    matrix->m = (int16_t **)malloc(rows * sizeof(int16_t *));
    for(uint16_t row = 0; row < rows; row++)
    {
        matrix->m[row] = (int16_t *)malloc(columns * sizeof(int16_t));
        for(uint16_t column = 0; column < columns; column++)
        {
            matrix->m[row][column] = 0;
        }
    }
    matrix->numOfRows = rows;
    matrix->numOfCols = columns;
}

void freeMatrix16(Matrix16_t * matrix)
{
    for(uint16_t row = 0; row < matrix->numOfRows; row++)
    {
        free(matrix->m[row]);
    }
    free(matrix->m);
}

void addRowMatrix16(int16_t * row, uint16_t index, Matrix16_t * matrix)
{
    for(uint16_t column = 0; column < matrix->numOfCols; column++)
    {
        matrix->m[index][column] = row[column];
    }
}

void allocateMatrix32(uint16_t rows, uint16_t columns, Matrix32_t * matrix)
{
    matrix->m = (int32_t **)malloc(rows * sizeof(int32_t *));
    for(uint16_t row = 0; row < rows; row++)
    {
        matrix->m[row] = (int32_t *)malloc(columns * sizeof(int32_t));
        for(uint16_t column = 0; column < columns; column++)
        {
            matrix->m[row][column] = 0;
        }
    }
    matrix->numOfRows = rows;
    matrix->numOfCols = columns;
}

void freeMatrix32(Matrix32_t * matrix)
{
    for(uint16_t row = 0; row < matrix->numOfRows; row++)
    {
        free(matrix->m[row]);
    }
    free(matrix->m);
}

void addRowMatrix32(int32_t * row, uint16_t index, Matrix32_t * matrix)
{
    for(uint16_t column = 0; column < matrix->numOfCols; column++)
    {
        matrix->m[index][column] = row[column];
    }
}

void multiplyMatrix16(Matrix16_t * m1, Matrix16_t * m2, Matrix32_t * mOut)
{
    assert(m1->numOfCols == m2->numOfRows);
    allocateMatrix32(m1->numOfRows, m2->numOfCols, mOut);
    int32_t dotProduct = 0;
    for(uint16_t row = 0; row < m1->numOfRows; row++)
    {
        for(uint16_t column = 0; column < m2->numOfCols; column++)
        {
            dotProduct = 0;
            for(uint16_t index = 0; index < m2->numOfRows; index++)
            {
                dotProduct += m1->m[row][index] * m2->m[index][column];
            }
            mOut->m[row][column] = dotProduct;
        }
    }
}

void printMatrix16(Matrix16_t * matrix)
{
    for(uint16_t row = 0; row < matrix->numOfRows; row++)
    {
        for(uint16_t column = 0; column < matrix->numOfCols; column++)
        {
            ESP_LOGI(TAG, "%d ", matrix->m[row][column]);
        }
        ESP_LOGI(TAG, "\n");
    }
}

void printMatrix32(Matrix32_t * matrix)
{
    for(uint16_t row = 0; row < matrix->numOfRows; row++)
    {
        for(uint16_t column = 0; column < matrix->numOfCols; column++)
        {
            ESP_LOGI(TAG, "%ld ", matrix->m[row][column]);
        }
        ESP_LOGI(TAG, "\n");
    }
}