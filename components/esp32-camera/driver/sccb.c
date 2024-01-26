/*
 * This file is part of the OpenMV project.
 * Copyright (c) 2013/2014 Ibrahim Abdelkader <i.abdalkader@gmail.com>
 * This work is licensed under the MIT license, see the file LICENSE for details.
 *
 * SCCB (I2C like) driver.
 *
 */
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sccb.h"
#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
#include "esp32-hal-log.h"
#else
#include "esp_log.h"
static const char* TAG = "sccb";
#endif

#define LITTLETOBIG(x)          ((x<<8)|(x>>8))

#include "driver/i2c_master.h"

#define SCCB_FREQ               100000           /*!< I2C master frequency*/
#define WRITE_BIT               I2C_MASTER_WRITE /*!< I2C master write */
#define READ_BIT                I2C_MASTER_READ  /*!< I2C master read */
#define ACK_CHECK_EN            0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS           0x0              /*!< I2C master will not check ack from slave */
#define ACK_VAL                 0x0              /*!< I2C ack value */
#define NACK_VAL                0x1              /*!< I2C nack value */
#if CONFIG_SCCB_HARDWARE_I2C_PORT1
const int SCCB_I2C_PORT         = 1;
#else
const int SCCB_I2C_PORT         = 0;
#endif
static uint8_t ESP_SLAVE_ADDR   = 0x3c;

static i2c_master_dev_handle_t dev_handle;
static i2c_master_bus_handle_t bus_handle;

int SCCB_Init(int pin_sda, int pin_scl)
{
    
    ESP_LOGI(TAG, "pin_sda %d pin_scl %d\n", pin_sda, pin_scl);

    i2c_master_bus_config_t i2c_mst_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .i2c_port = SCCB_I2C_PORT,
        .scl_io_num = pin_scl,
        .sda_io_num = pin_sda,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ESP_SLAVE_ADDR,
        .scl_speed_hz = SCCB_FREQ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));

    return 0;
}

void SCCB_Deinit()
{
    i2c_master_bus_rm_device(dev_handle);
    i2c_del_master_bus(bus_handle);
}

uint8_t SCCB_Set_Device(uint8_t slave_addr)
{
    if (ESP_SLAVE_ADDR != slave_addr) {
        ESP_ERROR_CHECK(i2c_master_bus_rm_device(dev_handle));
        i2c_device_config_t dev_cfg = {
            .dev_addr_length = I2C_ADDR_BIT_LEN_7,
            .device_address = slave_addr,
            .scl_speed_hz = SCCB_FREQ,
        };
        ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
        ESP_SLAVE_ADDR = slave_addr;
    }
    return ESP_SLAVE_ADDR;
}

uint8_t SCCB_Probe()
{
    uint8_t slave_addr = 0x0;
    while(slave_addr < 0x7f) {
        esp_err_t err = i2c_master_probe(bus_handle, slave_addr, -1);
        if(err == ESP_OK) {
            return SCCB_Set_Device(slave_addr);
        }
        slave_addr++;
    }
    return ESP_SLAVE_ADDR;
}

uint8_t SCCB_Read(uint8_t slv_addr, uint8_t reg)
{
    uint8_t data=0;

    SCCB_Set_Device(slv_addr);

    ESP_LOGI(TAG, "Read %d", reg);

    esp_err_t err = i2c_master_transmit_receive(
        dev_handle,
        (const uint8_t *)&reg,
        2,
        &data,
        2,
        -1
    );

    if(err != ESP_OK) {
        ESP_LOGE(TAG, "SCCB_Read Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, err);
    }
    return data;
}

uint8_t SCCB_Write(uint8_t slv_addr, uint8_t reg, uint8_t data)
{
    uint8_t write_buffer[2] = { reg, data };

    SCCB_Set_Device(slv_addr);

    ESP_LOGI(TAG, "Wrote %d", reg);

    esp_err_t ret = i2c_master_transmit(
        dev_handle,
        (const uint8_t *)&write_buffer,
        2,
        -1
    );

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "SCCB_Write Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return ret == ESP_OK ? 0 : -1;
}

uint8_t SCCB_Read16(uint8_t slv_addr, uint16_t reg)
{
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *addr = (uint8_t *)&reg_htons;
    uint8_t buffer = 0;

    SCCB_Set_Device(slv_addr);

    esp_err_t ret = i2c_master_transmit_receive(
        dev_handle,
        addr,
        2,
        &buffer,
        2,
        -1
    );

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "W [%04x]=%d fail\n", reg, buffer);
    }

    return buffer;
}

uint8_t SCCB_Write16(uint8_t slv_addr, uint16_t reg, uint8_t data)
{
    int len = 3;
    uint16_t reg_htons = LITTLETOBIG(reg);
    uint8_t *addr = (uint8_t *)&reg_htons;
    uint8_t write_buffer[3] = { addr[0], addr[1], data }; 

    SCCB_Set_Device(slv_addr);

    esp_err_t ret = i2c_master_transmit(
        dev_handle,
        &write_buffer,
        len,
        -1
    );

    if(ret != ESP_OK) {
        ESP_LOGE(TAG, "SCCB_Write Failed addr:0x%02x, reg:0x%02x, data:0x%02x, ret:%d", slv_addr, reg, data, ret);
    }
    return ret == ESP_OK ? 0 : -1;
}
