file(GLOB_RECURSE SOURCES ./*.c)
idf_component_register(SRCS ${SOURCES}
		INCLUDE_DIRS include
    REQUIRES esp_driver_gpio esp_http_server nvs_flash wifi_driver esp32-camera driver ip5306_driver mpu6886_driver i2c_master esp_timer)