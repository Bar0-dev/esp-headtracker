idf_component_register(SRCS "imu_spi.c" "imu_i2c.c" "imu_helpers.c" "imu_hal.c" "imu_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_timer esp_ao events_broker nvs_helpers core)
