#ifndef NVS_HELPERS_H
#define NVS_HELPERS_H

#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#define MAX_KEY_LENGTH 20

typedef enum
{
    ACCEL_SCALE_X,
    ACCEL_BIAS_X,
    ACCEL_SCALE_Y,
    ACCEL_BIAS_Y,
    ACCEL_SCALE_Z,
    ACCEL_BIAS_Z,
    LAST_KEY,
} AccelKeys_t;

typedef char KeyString_t[MAX_KEY_LENGTH];

static const KeyString_t accelKeyStrings[LAST_KEY] = { "accel_scale_X", "accel_bias_X", "accel_scale_Y", "accel_bias_Y", "accel_scale_Z", "accel_bias_Z"};
static const KeyString_t offsetsNamespace = "offsets";

void get_accel_offsets(int16_t scales[], int16_t biases[]);
void store_accel_offsets(int16_t scales[], int16_t biases[]);
#endif // NVS_HELPERS_H