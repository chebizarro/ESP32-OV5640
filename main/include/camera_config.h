/***********************************************************************************************************
 * 优仕德电子
 * @file    main.c
 * @brief   基于ESP32 IDF4.1，ESP32-CAM PSRAM S003摄像头接口配置文件
 * @date    2021-1-30
 * @note    ESP32-CAM PSRAM 模组摄像头初始化参数设置
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。 
 ***********************************************************************************************************/

#ifndef __CAMERA_CONFIG__
#define __CAMERA_CONFIG__

#include "esp_camera.h"
#include "S003_config.h"

//摄像头引脚定义
#define CAM_PIN_PWDN        -1  //power down is not used
#define CAM_PIN_RESET       S003_CAMERA_RESET_IO //software reset will be performed
#define CAM_PIN_XCLK        S003_CAMERA_XCLK_IO
#define CAM_PIN_SIOD        S003_CAMERA_SIOD_IO
#define CAM_PIN_SIOC        S003_CAMERA_SIOC_IO

#define CAM_PIN_D7          S003_CAMERA_D7_IO
#define CAM_PIN_D6          S003_CAMERA_D6_IO
#define CAM_PIN_D5          S003_CAMERA_D5_IO
#define CAM_PIN_D4          S003_CAMERA_D4_IO
#define CAM_PIN_D3          S003_CAMERA_D3_IO
#define CAM_PIN_D2          S003_CAMERA_D2_IO
#define CAM_PIN_D1          S003_CAMERA_D1_IO
#define CAM_PIN_D0          S003_CAMERA_D0_IO
#define CAM_PIN_VSYNC       S003_CAMERA_VSYNC_IO
#define CAM_PIN_HREF        S003_CAMERA_HREF_IO
#define CAM_PIN_PCLK        S003_CAMERA_PCLK_IO

// 摄像头配置
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, //YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_VGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 2       //if more than one, i2s runs in continuous mode. Use only with JPEG
};

#endif // __CAMERA_CONFIG__
