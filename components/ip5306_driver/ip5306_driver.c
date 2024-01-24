/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    ip5306_driver.c
 * @version ip5306_driver v1.0
 * @brief   基于ESP32 IDF4.1， ip5306电池管理芯片驱动 接口函数实现。
 * @date    2021-1-30
 * @note    
 *          ->提供一个读电池电量接口，分辨率为25%。
 *          ->提供一个判断是否充电接口。
 *          ->提供一个判断是否充满电接口。
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。
 ***********************************************************************************************************/

#include "ip5306_driver.h"
#include "../i2c_master/include/i2c_master.h"
#include "driver/i2c_master.h"
#include <stdio.h>

static const char *TAG = "IP5306";

static i2c_master_dev_handle_t dev_handle;


esp_err_t ip5306_i2c_read(uint8_t address, uint8_t* result, uint8_t length) {
    bool success = false;
    uint8_t retries = 3;

    while ((success == false) && (retries > 0))
    {
        esp_err_t err = i2c_master_transmit_receive(
          dev_handle,
          (const uint8_t *)&address,
          1,
          result, length,
          -1
        );

        if (err == ESP_OK) {
            success = true;
        } else {
            //ESP_LOGW(TAG, "Failed to read device at address: %X", address);
            retries--;
            esp_rom_delay_us( 500 );
        }
    }

    return success ? ESP_OK : ESP_ERR_INVALID_ARG;
}


/**
  * @brief  Ip5306读电量
  * @param  void
  * @retval 返回电量值
  */
uint8_t Ip5306_Check_Power(void)
{
    uint8_t data = 0;
    //Ip5306_I2c_Read(IP5306_CHECK_POWER, 1, &data);
    ip5306_i2c_read(IP5306_CHECK_POWER, &data, 1);
    if(data == 0x00)
        return 100;
    else if(data == 0x80)
        return 75;
    else if(data == 0xC0)
        return 50;
    else if(data == 0xE0)
        return 25;
    else
        return 0;
}

/**
  * @brief  Ip5306判断是否在充电
  * @param  void
  * @retval 1：充电中 0：未充电
  */
uint8_t Ip5306_Check_Charge(void)
{
    uint8_t data = 0;
    //Ip5306_I2c_Read(IP5306_CHECK_CHANGE, 1, &data);
    ip5306_i2c_read(IP5306_CHECK_CHANGE, &data, 1);
    if(data>>3 & 0x01){
      return 1;
    }
    return 0;
}

/**
  * @brief  Ip5306判断是否充满。
  * @param  void
  * @retval 1：已充满 0：未充满
  */
uint8_t Ip5306_Check_Full(void)
{
    uint8_t data = 0;
    //Ip5306_I2c_Read(IP5306_CHECK_FULL, 1, &data);
    ip5306_i2c_read(IP5306_CHECK_FULL, &data, 1);
    if(data>>3 & 0x01){
      return 1;
    }
    return 0;
}