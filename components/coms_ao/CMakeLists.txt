
idf_component_register(SRCS "coms_ao.c" "wifi_helpers.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_ao events_broker)