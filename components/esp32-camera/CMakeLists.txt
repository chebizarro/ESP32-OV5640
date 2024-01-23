if(IDF_TARGET STREQUAL "esp32")
  set(COMPONENT_SRCS
    driver/camera.c
    driver/sccb.c
    driver/sensor.c
    driver/xclk.c
    sensors/ov2640.c
    sensors/ov3660.c
    sensors/ov5640.c
    sensors/ov7725.c
    sensors/ov7670.c
    sensors/nt99141.c
    conversions/yuv.c
    conversions/to_jpg.cpp
    conversions/to_bmp.c
    conversions/jpge.cpp
    conversions/esp_jpg_decode.c
    )

  set(COMPONENT_ADD_INCLUDEDIRS
    driver/include
    conversions/include
    )

  set(COMPONENT_PRIV_INCLUDEDIRS
    driver/private_include
    sensors/private_include
    conversions/private_include
    )

  set(COMPONENT_REQUIRES driver esp_psram esp_driver_gpio esp_timer esp_hw_support)
  set(COMPONENT_PRIV_REQUIRES soc freertos nvs_flash esp_driver_gpio esp_timer esp_hw_support esp_rom)

  register_component()
endif()
