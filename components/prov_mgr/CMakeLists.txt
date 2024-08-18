idf_component_register(SRCS "prov_mgr.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi nvs_helpers wifi_provisioning)