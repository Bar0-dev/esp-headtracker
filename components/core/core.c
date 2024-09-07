#include "core.h"
#include "esp_log.h"
#include <stdlib.h>

const static char *TAG = "CORE";

void allocateMatrix16(uint16_t rows, uint16_t columns, Matrix16_t *matrix) {
  matrix->m = (int16_t **)malloc(rows * sizeof(int16_t *));
  for (uint16_t row = 0; row < rows; row++) {
    matrix->m[row] = (int16_t *)malloc(columns * sizeof(int16_t));
    for (uint16_t column = 0; column < columns; column++) {
      matrix->m[row][column] = 0;
    }
  }
  matrix->numOfRows = rows;
  matrix->numOfCols = columns;
}

void freeMatrix16(Matrix16_t *matrix) {
  for (uint16_t row = 0; row < matrix->numOfRows; row++) {
    free(matrix->m[row]);
  }
  free(matrix->m);
}

void addRowMatrix16(int16_t *row, uint16_t index, Matrix16_t *matrix) {
  for (uint16_t column = 0; column < matrix->numOfCols; column++) {
    matrix->m[index][column] = row[column];
  }
}

void addColumnMatrix16(int16_t *column, uint16_t index, Matrix16_t *matrix) {
  for (uint16_t row = 0; row < matrix->numOfRows; row++) {
    matrix->m[row][index] = column[row];
  }
}

void allocateMatrix32(uint16_t rows, uint16_t columns, Matrix32_t *matrix) {
  matrix->m = (int32_t **)malloc(rows * sizeof(int32_t *));
  for (uint16_t row = 0; row < rows; row++) {
    matrix->m[row] = (int32_t *)malloc(columns * sizeof(int32_t));
    for (uint16_t column = 0; column < columns; column++) {
      matrix->m[row][column] = 0;
    }
  }
  matrix->numOfRows = rows;
  matrix->numOfCols = columns;
}

void freeMatrix32(Matrix32_t *matrix) {
  for (uint16_t row = 0; row < matrix->numOfRows; row++) {
    free(matrix->m[row]);
  }
  free(matrix->m);
}

void addRowMatrix32(int32_t *row, uint16_t index, Matrix32_t *matrix) {
  for (uint16_t column = 0; column < matrix->numOfCols; column++) {
    matrix->m[index][column] = row[column];
  }
}

void multiplyMatrix16(Matrix16_t *m1, Matrix16_t *m2, Matrix32_t *mOut) {
  assert(m1->numOfCols == m2->numOfRows);
  allocateMatrix32(m1->numOfRows, m2->numOfCols, mOut);
  int32_t dotProduct = 0;
  for (uint16_t row = 0; row < m1->numOfRows; row++) {
    for (uint16_t column = 0; column < m2->numOfCols; column++) {
      dotProduct = 0;
      for (uint16_t index = 0; index < m2->numOfRows; index++) {
        dotProduct += m1->m[row][index] * m2->m[index][column];
      }
      mOut->m[row][column] = dotProduct;
    }
  }
}

void printMatrix16(Matrix16_t *matrix) {
  for (uint16_t row = 0; row < matrix->numOfRows; row++) {
    for (uint16_t column = 0; column < matrix->numOfCols; column++) {
      ESP_LOGI(TAG, "%d ", matrix->m[row][column]);
    }
    ESP_LOGI(TAG, "\n");
  }
}

void printMatrix32(Matrix32_t *matrix) {
  for (uint16_t row = 0; row < matrix->numOfRows; row++) {
    for (uint16_t column = 0; column < matrix->numOfCols; column++) {
      ESP_LOGI(TAG, "%ld ", matrix->m[row][column]);
    }
    ESP_LOGI(TAG, "\n");
  }
}

void transposeMatrix16(Matrix16_t *matrix, Matrix16_t *matrixTransposed) {
  allocateMatrix16(matrix->numOfCols, matrix->numOfRows, matrixTransposed);
  for (uint16_t column = 0; column < matrixTransposed->numOfCols; column++) {
    addColumnMatrix16(matrix->m[column], column, matrixTransposed);
  }
}

int16_t determinant2x2Matrix16(Matrix16_t *matrix) {
  assert((matrix->numOfCols = 2) & (matrix->numOfRows = 2));
  // Leibniz formula
  int16_t a = matrix->m[0][0];
  int16_t b = matrix->m[0][1];
  int16_t c = matrix->m[1][0];
  int16_t d = matrix->m[1][1];
  return a * d - b * c;
}

int16_t determinant3x3Matrix16(Matrix16_t *matrix) {
  assert((matrix->numOfCols == 3) & (matrix->numOfRows == 3));
  // Leibniz formula
  int16_t a = matrix->m[0][0];
  int16_t b = matrix->m[0][1];
  int16_t c = matrix->m[0][2];
  int16_t d = matrix->m[1][0];
  int16_t e = matrix->m[1][1];
  int16_t f = matrix->m[1][2];
  int16_t g = matrix->m[2][0];
  int16_t h = matrix->m[2][1];
  int16_t i = matrix->m[2][2];
  return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
}

int16_t minorOf3x3Matrix16(uint8_t row, uint8_t column, Matrix16_t *matrix) {
  Matrix16_t minorMatrix;
  allocateMatrix16(2, 2, &minorMatrix);
  uint8_t mmRow = 0;
  uint8_t mmColumn = 0;
  for (uint16_t rowIndex = 0; rowIndex < matrix->numOfRows; rowIndex++) {
    mmColumn = 0;
    if (rowIndex == row) {
      continue;
    }
    for (uint16_t columnIndex = 0; columnIndex < matrix->numOfCols;
         columnIndex++) {
      if (columnIndex == column) {
        continue;
      }
      minorMatrix.m[mmRow][mmColumn] = matrix->m[rowIndex][columnIndex];
      mmColumn++;
    }
    mmRow++;
  }
  int16_t determinant = determinant2x2Matrix16(&minorMatrix);
  freeMatrix16(&minorMatrix);
  return determinant;
}

void scaledInverse3x3Matrix16(Matrix16_t *matrix, Matrix16_t *matrixInverse) {
  int16_t determinant = determinant3x3Matrix16(matrix);
  assert(determinant != 0);
  Matrix16_t signMatrix;
  Matrix16_t cofactorMatrix;
  allocateMatrix16(3, 3, matrixInverse);
  allocateMatrix16(3, 3, &signMatrix);
  allocateMatrix16(3, 3, &cofactorMatrix);
  int16_t row1[] = {1, -1, 1};
  int16_t row2[] = {-1, 1, -1};
  addRowMatrix16(row1, 0, &signMatrix);
  addRowMatrix16(row2, 1, &signMatrix);
  addRowMatrix16(row1, 2, &signMatrix);
  for (uint16_t rowIndex = 0; rowIndex < matrix->numOfRows; rowIndex++) {
    for (uint16_t columnIndex = 0; columnIndex < matrix->numOfCols;
         columnIndex++) {
      cofactorMatrix.m[rowIndex][columnIndex] =
          INVERSE_MATRIX_SCALER * signMatrix.m[rowIndex][columnIndex] *
          minorOf3x3Matrix16(rowIndex, columnIndex, matrix) / determinant;
    }
  }
  transposeMatrix16(&cofactorMatrix, matrixInverse);
  freeMatrix16(&signMatrix);
  freeMatrix16(&cofactorMatrix);
}
