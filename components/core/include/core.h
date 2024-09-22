#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#define MAX_PACKET_SIZE 200
#define MAX_SINGLE_READING_SIZE 8
#define MAX_MAG_CALIBRATION_SAMPLES 800 // equivalent of 8s of calibration
#define BASE_MATRIX_NO_OF_COLUMNS 3
#define INVERSE_MATRIX_SCALER 10000

typedef struct {
  uint8_t length;
  char payload[MAX_PACKET_SIZE];
} Packet_t;

typedef enum { X_AXIS, Y_AXIS, Z_AXIS, NO_AXIS } Axis_t;

typedef enum { ACCEL, GYRO, MAG, NO_SENSOR } Sensor_t;

typedef enum { POSITIVE, NEGATIVE, NO_DIRECTION } Direction_t;

typedef int16_t Vector16_t[NO_AXIS];
typedef int32_t Vector32_t[NO_AXIS];

typedef struct {
  int16_t **m;
  uint16_t numOfRows;
  uint16_t numOfCols;
} Matrix16_t;

typedef struct {
  int32_t **m;
  uint16_t numOfRows;
  uint16_t numOfCols;
} Matrix32_t;

void allocateMatrix16(uint16_t rows, uint16_t columns, Matrix16_t *matrix);
void freeMatrix16(Matrix16_t *matrix);
void addRowMatrix16(int16_t *row, uint16_t index, Matrix16_t *matrix);
void addColumnMatrix16(int16_t *column, uint16_t index, Matrix16_t *matrix);
void transposeMatrix16(Matrix16_t *matrix, Matrix16_t *matrixTransposed);
int16_t determinant3x3Matrix16(Matrix16_t *matrix);
int16_t minorOf3x3Matrix16(uint8_t row, uint8_t column, Matrix16_t *matrix);
void scaledInverse3x3Matrix16(Matrix16_t *matrix, Matrix16_t *matrixInverse);
void inverse3x3Matrix16(Matrix16_t *matrix, Matrix16_t *matrixInverse);
void printMatrix16(Matrix16_t *matrix);
void multiplyMatrix16(Matrix16_t *m1, Matrix16_t *m2, Matrix32_t *mOut);
void allocateMatrix32(uint16_t rows, uint16_t columns, Matrix32_t *matrix);
void addRowMatrix32(int32_t *row, uint16_t index, Matrix32_t *matrix);
void freeMatrix32(Matrix32_t *matrix);
void printMatrix32(Matrix32_t *matrix);

#endif
