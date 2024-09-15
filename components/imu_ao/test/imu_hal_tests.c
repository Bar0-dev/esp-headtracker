#include "../imu_hal.h"
#include "test_helpers.h"
#include "unity.h"
#include <stdint.h>

TEST_CASE("When mpu is configured", "[mpuConf]") { imu_hal_init(); }
TEST_CASE("Meausre sensor read timing", "[perf]") {
  ImuData_t data;
  measure_execution_time1(&imu_read, *data, "imu_read");
}
