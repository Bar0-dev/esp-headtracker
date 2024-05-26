idf_component_register(SRCS "led_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    PRIV_REQUIRES esp_ao events_broker)
