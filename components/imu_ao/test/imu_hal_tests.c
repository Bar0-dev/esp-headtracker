#include "../imu_hal.h"
#include "imu_ao.h"
#include "test_helpers.h"
#include "unity.h"
#include <stdint.h>

static Imu imu;
Active *AO_Imu = &imu.super;

TEST_CASE("When mpu is configured", "[mpuConf]") { imu_hal_init(); }

TEST_CASE("Meausre sensor read timing", "[perf]") {
  imu_hal_init_dbuffer();
  measure_execution_time(imu_hal_update_dbuffer, "imu_hal_update_dbuffer");
}
