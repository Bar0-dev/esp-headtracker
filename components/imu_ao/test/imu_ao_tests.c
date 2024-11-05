#include "esp_ao.h"
#include "imu_ao.h"
#include "test_helpers.h"
#include "unity.h"
#include <stdint.h>

static Broker broker;
Active *AO_Broker = &broker.super;
static Imu imu;
Active *AO_Imu = &imu.super;

TEST_CASE("When accel calibration is performed", "[calibration_states]") {
  Imu_ctor(&imu);
  Active_start(AO_Imu, "Imu thread", 4096, 21, 1, 20);
  Broker_ctor(&broker);
  Active_start(AO_Broker, "Broker thread", 4096, 20, tskNO_AFFINITY, 20);
  Event evt = {.sig = EV_CONTROLLER_CALIBRATE_ACCEL, .payload = (void *)0};
  Active_post(AO_Imu, &evt);
}
//
// TEST_CASE("Meausre sensor read timing", "[performance]") {
//   imu_hal_init_dbuffer();
//   measure_execution_time(imu_hal_update_dbuffer, "imu_hal_update_dbuffer");
// }
