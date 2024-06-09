#include "nvs_helpers.h"

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

static void get_int16(nvs_handle_t *handle, const KeyString_t key, int16_t *val){
    esp_err_t err = nvs_get_i16(*handle, key, val);
    if(err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "\nThe value is not initialized yet!\n");
    }
    ESP_LOGI(TAG, "Value was read: %d", *val);
}

void get_accel_offsets(int16_t scales[], int16_t biases[]){
    nvs_handle_t nvs_handle;
    init_nvs(&nvs_handle, offsetsNamespace);
    int16_t scale;
    int16_t bias;
    for(uint8_t key = ACCEL_SCALE_X; key<LAST_KEY; key+=2){

        get_int16(&nvs_handle, accelKeyStrings[key], &scale);
        get_int16(&nvs_handle, accelKeyStrings[key+1], &bias);
        scales[key>>1] = scale;
        biases[key>>1] = bias;
    }
    nvs_close(nvs_handle);
}

void store_accel_offsets(int16_t scales[], int16_t biases[]){
    nvs_handle_t nvs_handle;
    init_nvs(&nvs_handle, offsetsNamespace);
    for(uint8_t key = ACCEL_SCALE_X; key<LAST_KEY; key+=2){
        ESP_ERROR_CHECK(nvs_set_i16(nvs_handle, accelKeyStrings[key], scales[key>>1]));
        ESP_ERROR_CHECK(nvs_set_i16(nvs_handle, accelKeyStrings[key+1], biases[key>>1]));
    }
    ESP_ERROR_CHECK(nvs_commit(nvs_handle));
    nvs_close(nvs_handle);
}