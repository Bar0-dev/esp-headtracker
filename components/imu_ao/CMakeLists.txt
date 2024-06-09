idf_component_register(SRCS "imu_hal.c" "imu_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_ao events_broker nvs_helpers)
