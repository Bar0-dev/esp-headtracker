#include "test_helpers.h"
#include "core.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdint.h>

uint32_t measure_execution_time(function_ptr_t func, char *funcName) {
  int64_t start_time = esp_timer_get_time();

  func();

  int64_t end_time = esp_timer_get_time();

  int64_t duration = end_time - start_time;

  ESP_LOGI("TEST_HELPERS", "Function %s execution time: %lld microseconds",
           funcName, duration);
  return duration;
}

uint32_t measure_execution_time1(function1_ptr_t func, void *args,
                                 char *funcName) {
  int64_t start_time = esp_timer_get_time();

  func(args);

  int64_t end_time = esp_timer_get_time();

  int64_t duration = end_time - start_time;

  ESP_LOGI("TEST_HELPERS", "Function %s execution time: %lld microseconds",
           funcName, duration);
  return duration;
}

uint32_t measure_execution_time2(function2_ptr_t func, void *arg1, void *arg2,
                                 char *funcName) {
  int64_t start_time = esp_timer_get_time();

  func(arg1, arg2);

  int64_t end_time = esp_timer_get_time();

  int64_t duration = end_time - start_time;

  ESP_LOGI("TEST_HELPERS", "Function %s execution time: %lld microseconds",
           funcName, duration);
  return duration;
}

void set_buffer_instance(Buffer_t *buffer, ImuData_t read) {
  for (uint8_t index = 0; index < MAX_BUFFER_SIZE; index++) {
    for (Sensor_t sensor = ACCEL; sensor < NO_SENSOR; sensor++) {
      for (Axis_t axis = X_AXIS; axis < NO_AXIS; axis++) {
        buffer->data[index].read[sensor][axis] = read[sensor][axis];
      }
    }
  }
  buffer->length = MAX_BUFFER_SIZE;
}

void calculateScaleBiasTest(int16_t x1, int16_t y1, int16_t x2, int16_t y2,
                            int16_t *scale, int16_t *bias) {
  float lscale = (float)(y2 - y1) / (float)(x2 - x1);
  printf("lscale: %f\n", lscale);
  *scale = (int16_t)(1000 * lscale);
  *bias = y1 - lscale * x1;
}
