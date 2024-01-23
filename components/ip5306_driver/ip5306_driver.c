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
#include <stdio.h>

/**
  * @brief  Ip5306读数据
  * @param  reg_addr:Ip5306寄存器地址; 
  * @param  len:读出数据的长度; 
  * @param  data_buf:待读数据的首地址 
  * @retval 参考esp_err_t
  */
static uint8_t Ip5306_I2c_Read(uint8_t reg_addr, uint8_t len, uint8_t *data_buf)
{
    int ret = 0;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (IP5306_ADDR << 1)|WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg_addr, ACK_CHECK_EN);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (IP5306_ADDR << 1)|READ_BIT, ACK_CHECK_EN);
    i2c_master_read(cmd, data_buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(IP5306_I2C_MASTER_NUM, cmd, 100/portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    I2c_Check_Err(ret);
    return ret;
}

/**
  * @brief  Ip5306读电量
  * @param  void
  * @retval 返回电量值
  */
uint8_t Ip5306_Check_Power(void)
{
    uint8_t data = 0;
    Ip5306_I2c_Read(IP5306_CHECK_POWER, 1, &data);
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
    Ip5306_I2c_Read(IP5306_CHECK_CHANGE, 1, &data);
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
    Ip5306_I2c_Read(IP5306_CHECK_FULL, 1, &data);
    if(data>>3 & 0x01){
      return 1;
    }
    return 0;
}