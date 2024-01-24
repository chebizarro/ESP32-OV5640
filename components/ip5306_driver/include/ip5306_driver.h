/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    ip5306_driver.h
 * @version ip5306_driver v1.0
 * @brief   基于ESP32 IDF4.1， ip5306电池管理芯片驱动 接口头文件。
 * @date    2021-1-30
 * @note    
 *          ->提供一个读电池电量接口，分辨率为25%。
 *          ->提供一个判断是否充电接口。
 *          ->提供一个判断是否充满电接口。
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。
 ***********************************************************************************************************/

#ifndef __IP5306_DRIVER_H
#define __IP5306_DRIVER_H

#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../main/include/S003_config.h"

#define IP5306_I2C_SCL_IO            S003_IP5306_SCL_IO
#define IP5306_I2C_SDA_IO            S003_IP5306_SDA_IO 
#define IP5306_I2C_MASTER_NUM        S003_IP5306_I2C_NUM 
#define IP5306_ADDR                  S003_IP5306_I2C_ADDR       //IP5306IIC地址

#define IP5306_CHECK_POWER 			     0x78       //IP5306读电量寄存器
#define IP5306_CHECK_CHANGE 		     0x70       //IP5306读是否充电寄存器
#define IP5306_CHECK_FULL            0x71       //IP5306读是否充满寄存器

 /**
  * @brief  Ip5306读电量
  * @param  void
  * @retval 返回电量值
  */
uint8_t Ip5306_Check_Power(void);

/**
  * @brief  Ip5306判断是否在充电
  * @param  void
  * @retval 1：充电中 0：未充电
  */
uint8_t Ip5306_Check_Charge(void);

/**
  * @brief  Ip5306判断是否充满。
  * @param  void
  * @retval 1：已充满 0：未充满
  */
uint8_t Ip5306_Check_Full(void);


#endif /* __IP5306_DRIVER_H */