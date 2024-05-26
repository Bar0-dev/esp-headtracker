idf_component_register(SRCS "polling_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_ao driver events_broker)