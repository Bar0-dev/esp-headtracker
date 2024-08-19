idf_component_register(SRCS "coms_ao.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_ao prov_mgr events_broker udp_client)