/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    S003_config.h
 * @brief   基于ESP32 IDF4.1，ESP32-CAM PSRAM (S003)硬件配置文件
 * @date    2021-1-28
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。 
 ***********************************************************************************************************/

#ifndef __S003_CONFIG_H_
#define __S003_CONFIG_H_

#include "driver/gpio.h"
#include "driver/i2c_master.h"

// I2C0
#define S003_I2C0_SCL_IO                    GPIO_NUM_4
#define S003_I2C0_SDA_IO                    GPIO_NUM_13
#define S003_I2C0_I2C_NUM                   I2C_NUM_0

// LED
#define S003_LED_IO                         GPIO_NUM_32

// IP5306
#define S003_IP5306_SCL_IO                  S003_I2C0_SCL_IO
#define S003_IP5306_SDA_IO                  S003_I2C0_SDA_IO
#define S003_IP5306_I2C_NUM                 S003_I2C0_I2C_NUM
#define S003_IP5306_I2C_ADDR                0x75

// MPU6886
#define S003_MPU6886_SCL_IO                 S003_I2C0_SCL_IO
#define S003_MPU6886_SDA_IO                 S003_I2C0_SDA_IO
#define S003_MPU6886_I2C_NUM                S003_I2C0_I2C_NUM
#define S003_MPU6886_I2C_ADDR               0x68

// PH2.0 Port
#define S003_PORT_1_IO                      GPIO_NUM_4
#define S003_PORT_2_IO                      GPIO_NUM_13

// Camera(ov5640)
#define S003_CAMERA_RESET_IO                GPIO_NUM_5
#define S003_CAMERA_XCLK_IO                 GPIO_NUM_15
#define S003_CAMERA_SIOD_IO                 GPIO_NUM_22
#define S003_CAMERA_SIOC_IO                 GPIO_NUM_23
#define S003_CAMERA_D7_IO                   GPIO_NUM_39
#define S003_CAMERA_D6_IO                   GPIO_NUM_34
#define S003_CAMERA_D5_IO                   GPIO_NUM_33
#define S003_CAMERA_D4_IO                   GPIO_NUM_27
#define S003_CAMERA_D3_IO                   GPIO_NUM_12
#define S003_CAMERA_D2_IO                   GPIO_NUM_35
#define S003_CAMERA_D1_IO                   GPIO_NUM_14
#define S003_CAMERA_D0_IO                   GPIO_NUM_2
#define S003_CAMERA_VSYNC_IO                GPIO_NUM_18
#define S003_CAMERA_HREF_IO                 GPIO_NUM_36
#define S003_CAMERA_PCLK_IO                 GPIO_NUM_26

#endif /* __S003_CONFIG_H_ */
