#include "test_helpers.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdint.h>
#include <stdio.h>

void measure_execution_time(function_ptr_t func, char *funcName) {
  int64_t start_time = esp_timer_get_time();

  func();

  int64_t end_time = esp_timer_get_time();

  int64_t duration = end_time - start_time;

  ESP_LOGI("TEST_HELPERS", "Function %s execution time: %lld microseconds",
           funcName, duration);
}

void measure_execution_time1(function1_ptr_t func, void *args, char *funcName) {
  int64_t start_time = esp_timer_get_time();

  func(args);

  int64_t end_time = esp_timer_get_time();

  int64_t duration = end_time - start_time;

  ESP_LOGI("TEST_HELPERS", "Function %s execution time: %lld microseconds",
           funcName, duration);
}
