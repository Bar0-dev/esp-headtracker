#include "nvs_helpers.h"

static const KeyString_t accelScaleKeys[NO_AXIS] = { "accel_scale_X", "accel_scale_Y", "accel_scale_Z"};
static const KeyString_t accelBiasKeys[NO_AXIS] = { "accel_bias_X", "accel_bias_Y", "accel_bias_Z"};
static const KeyString_t gyroBiasKeys[NO_AXIS] = { "gyro_bias_X", "gyro_bias_Y", "gyro_bias_Z"};
static const KeyString_t accelScaleNamespace = "accelScale";
static const KeyString_t accelBiasNamespace = "accelBias";
static const KeyString_t gyroBiasNamespace = "gyroBias";

static const char* TAG = "NVS";
static void init_nvs(nvs_handle_t *handle, const KeyString_t namespace)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGI(TAG, "\n\n NO FREE PAGES - ERASING FLASH!");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    ESP_ERROR_CHECK(nvs_open(namespace, NVS_READWRITE, handle));
}

static void get_int16(nvs_handle_t *handle, const KeyString_t key, int16_t *val)
{
    esp_err_t err = nvs_get_i16(*handle, key, val);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "\nThe value is not initialized yet!\n");
    }
    ESP_LOGI(TAG, "Value was read: %d", *val);
}

static void get_vector16(Vector16_t v, const KeyString_t namespace, const KeyString_t keys[])
{
    nvs_handle_t nvs_handle;
    init_nvs(&nvs_handle, namespace);
    int16_t val;
    for(uint8_t key = X_AXIS; key<NO_AXIS; key++){
        get_int16(&nvs_handle, keys[key], &val);
        v[key] = val;
    }
    nvs_close(nvs_handle);
}

static void set_vector16(Vector16_t v, const KeyString_t namespace, const KeyString_t keys[])
{
    nvs_handle_t nvs_handle;
    init_nvs(&nvs_handle, namespace);
    for(uint8_t key = X_AXIS; key<NO_AXIS; key++){
        ESP_ERROR_CHECK(nvs_set_i16(nvs_handle, keys[key], v[key]));
    }
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
}

void get_accel_scale_and_bias(Vector16_t scale, Vector16_t bias)
{
    get_vector16(scale, accelScaleNamespace, accelScaleKeys);
    get_vector16(bias, accelBiasNamespace, accelBiasKeys);
}

void set_accel_scale_and_bias(Vector16_t scale, Vector16_t bias)
{
    set_vector16(scale, accelScaleNamespace, accelScaleKeys);
    set_vector16(bias, accelBiasNamespace, accelBiasKeys);
}

void get_gyro_bias(Vector16_t bias)
{
    get_vector16(bias, gyroBiasNamespace, gyroBiasKeys);
}

void set_gyro_bias(Vector16_t bias)
{
    set_vector16(bias, gyroBiasNamespace, gyroBiasKeys);
}