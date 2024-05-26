idf_component_register(SRCS "imu_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver esp_ao events_broker)
