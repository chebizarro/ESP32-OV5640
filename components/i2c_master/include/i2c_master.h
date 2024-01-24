/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    i2c_master.h
 * @version i2c_master v1.0
 * @brief   基于ESP32 IDF4.1， i2c 总线主机配置驱动程序v1.0 接口头文件
 * @date    2020-12-7
 * @note    
 *          ->提供一个i2c总线初始化函数，不可改变速度
 *          ->提供一个i2c设备存在检测函数
 *          ->提供一个打印常用i2c设备通讯错误函数二次封装，封装了中文提示 
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。
 ***********************************************************************************************************/

#ifndef __I2C_MASTER_H
#define __I2C_MASTER_H

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "../../main/include/S003_config.h"

#define I2C_SCL_IO                  S003_I2C0_SCL_IO
#define I2C_SDA_IO                  S003_I2C0_SDA_IO
#define I2C_MASTER_NUM              S003_I2C0_I2C_NUM
#define I2C_MASTER_FREQ_HZ          400000

#define WRITE_BIT                   I2C_MASTER_WRITE
#define READ_BIT                    I2C_MASTER_READ 
#define ACK_CHECK_EN                0x01
#define ACK_CHECK_DIS               0x00
#define ACK_VAL                     0x00
#define NACK_VAL                    0x01

/**
  * @brief  初始化I2C主机
  * @param  I2c_num :i2c编号
  * @param  sda_num :sda引脚编号
  * @param  scl_num :scl引脚编号
  * @retval void
  */
void I2c_Master_Init(uint8_t I2c_num, uint8_t sda_num, uint8_t scl_num);


esp_err_t I2C_Add_Device(uint16_t dev_address, i2c_master_bus_handle_t dev_handle);


/**
  * @brief  检测设备是否存在
  * @param  I2c_addr:i2c设备地址
  * @param  test_val:用来测试的值，应防止与器件寄存器地址冲突
  * @retval 存在返回0，不存在返回1
  */
uint8_t I2C_ALIVE(uint8_t I2c_addr, uint8_t test_val);

/**
  * @brief  打印i2c cmd执行结果
  * @param  val:错误号
  * @retval void
  */
void I2c_Check_Err(esp_err_t val);

#endif /* __I2C_MASTER_H */
