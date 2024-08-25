#ifndef IMU_HELPERS_H
#define IMU_HELPERS_H

#include "imu_hal.h"
#include "packet.h"

void imu_log_data(ImuData_t data, SensorType_t sensor, bool convert);
void imu_process_data(ImuData_t data, packet_t *packet);
void imu_calc_scale_and_bias(int16_t scale[], int16_t bias[], int16_t accel_offsets[]);
void imu_apply_accel_offsets(ImuData_t data, int16_t scales[], int16_t bias[]);
#endif