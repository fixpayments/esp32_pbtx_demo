idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
		       REQUIRES esp32_pbtx_lib esp_http_client esp_wifi)
