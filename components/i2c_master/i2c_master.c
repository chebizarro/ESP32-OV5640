/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    i2c_master.c
 * @version i2c_master v1.0
 * @brief   基于ESP32 IDF4.1， i2c 总线主机配置驱动程序v1.0 接口实现文件
 * @date    2020-12-7
 * @note    
 *          ->提供一个i2c总线初始化函数，不可改变速度
 *          ->提供一个i2c设备存在检测函数
 *          ->提供一个打印常用i2c设备通讯错误函数二次封装，封装了中文提示
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。
 ***********************************************************************************************************/

#include "i2c_master.h"
#include "driver/i2c_master.h"
#include <stdio.h>

static i2c_master_bus_handle_t bus_handle;

/**
  * @brief  初始化I2C主机
  * @param  I2c_num :i2c编号
  * @param  sda_num :sda引脚编号
  * @param  scl_num :scl引脚编号
  * @retval void
  */
void I2c_Master_Init(uint8_t I2c_num, uint8_t sda_num, uint8_t scl_num)
{

    i2c_master_bus_config_t i2c_mst_config = {
            .clk_source = I2C_CLK_SRC_DEFAULT,
            .i2c_port = I2c_num,
            .scl_io_num = scl_num,
            .sda_io_num = sda_num,
            .glitch_ignore_cnt = 7,
            .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

}


esp_err_t read_16(uint8_t address, uint16_t* result) {
    bool success = false;
    uint8_t retries = 3;
    uint8_t addr[2] = {address };
    uint8_t buffer[2] = { 0, 0 };

    while ((success == false) && (retries > 0))
    {
        esp_err_t err = i2c_master_transmit_receive(bus_handle, addr, sizeof(addr), buffer, 2, -1);

        if (err == ESP_OK) {
            *result = ((uint16_t)buffer[0] << 8) | buffer[1];
            success = true;
        } else {
            ESP_LOGW(TAG, "Failed to read i2c device at address: %X", address);
            retries--;
            esp_rom_delay_us( 500 );
        }
    }

    return success ? ESP_OK : ESP_ERR_INVALID_ARG;
}

/**
  * @brief  检测设备是否存在
  * @param  I2c_addr:i2c设备地址
  * @param  test_val:用来测试的值，应防止与器件寄存器地址冲突
  * @retval 存在返回0，不存在返回1
  */
uint8_t I2C_ALIVE(uint8_t I2c_addr, uint8_t test_val)
{
  int ret = 0;
  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (I2c_addr << 1)|WRITE_BIT, ACK_CHECK_EN);
  i2c_master_write_byte(cmd, test_val, ACK_CHECK_EN);
  i2c_master_stop(cmd);
  ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 100/portTICK_PERIOD_MS);
  i2c_cmd_link_delete(cmd);
  I2c_Check_Err(ret);
  if(ret == ESP_OK) {
    return 0;
  }
  return 1;
}

/**
  * @brief  打印i2c cmd执行结果
  * @param  val:错误号
  * @retval void
  */
void I2c_Check_Err(esp_err_t val)
{
  switch(val){
    case ESP_OK:
      printf("ESP_OK 成功\n");
      break;
    case ESP_ERR_INVALID_ARG:
      printf("ESP_ERR_INVALID_ARG 参数错误\n");
      break;
    case ESP_FAIL:
      printf("ESP_FAIL 发送命令错误，从机未确认传输。\n");
      break;
    case ESP_ERR_INVALID_STATE:
      printf("ESP_ERR_INVALID_STATE I2C 驱动程序未安装或不在主模式下。\n");
      break;
    case ESP_ERR_TIMEOUT:
      printf("ESP_ERR_TIMEOUT 由于总线繁忙，操作超时。\n");
      break;
    default:
      break;
  }
}

