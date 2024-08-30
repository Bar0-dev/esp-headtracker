#ifndef NVS_HELPERS_H
#define NVS_HELPERS_H

#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "core.h"

#define MAX_KEY_LENGTH 20

typedef char KeyString_t[MAX_KEY_LENGTH];

void get_accel_scale_and_bias(Vector16_t scale, Vector16_t bias);
void set_accel_scale_and_bias(Vector16_t scale, Vector16_t bias);
void get_gyro_bias(Vector16_t bias);
void set_gyro_bias(Vector16_t bias);
#endif // NVS_HELPERS_H