#include "../imu_hal.h"
#include "unity.h"
#include <stdint.h>

TEST_CASE("When mpu is configured", "[mpuConf]") { imu_hal_init(); }
