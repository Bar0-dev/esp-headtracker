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
