/***********************************************************************************************************
 * 优仕德电子
 * @file    main.c
 * @brief   基于ESP32 IDF4.1，ESP32-CAM PSRAM S003摄像头模组测试程序入口
 * @date    2021-1-30
 * @note    本例程基于ESP32-CAM PSRAM S003摄像头模组(ov2640)测试
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。 
 ***********************************************************************************************************/

#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "spi_flash_mmap.h"

#include "esp_camera.h"
#include "camera_config.h"
#include "wifi_driver.h"
#include "camera_http.h"
#include "i2c_master.h"
#include "ip5306_driver.h"
#include "mpu6886_driver.h"

static char TAG[] = "ESP32-CAM PSRAM test";

// wifi模式选择 1:AP  2:STA  3:Smartconfig
#define WIFI_MODE       1

// wifi AP ID和密码设置
#define WIFI_AP_SSID    "ESP32-CAM"
#define WIFI_AP_PASS    "12345678"

// wifi STA ID和密码设置
#define WIFI_STA_SSID   "XXXXXXXX"
#define WIFI_STA_PASS   "XXXXXXXX"

// 是否开启MPU6886的测试
#define MPU6886_TEST    0

// 是否开启IP5306的测试  
#define IP5306_TEST     1
    
static void AppTask(void *pvParameter);
static void Led_Task_Test(void *pvParameter);

void app_main(void)
{   
    // 创建LED测试任务
    xTaskCreatePinnedToCore(Led_Task_Test, "Led_Task_Test", 1024, NULL, 6, NULL, 1);

    // nvs分区初始化(使用wifi需要用到)
    esp_err_t err = nvs_flash_init();
    if(ESP_OK != err){
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // wifi初始化
    WIFI_Init();

#if WIFI_MODE == 1
    // 初始化成ap模式(此模式下ESP32 IP 为 192.168.4.1)
    WIFI_Init_Softap(WIFI_AP_SSID, WIFI_AP_PASS); 
#elif WIFI_MODE == 2
    // 初始化为sta模式(填入你需要连接AP的 SSID 和 密码)
    WIFI_Init_Sta(WIFI_STA_SSID, WIFI_STA_PASS);
#else 
    // 使用smartconfig   
    WIFI_Smart_Config(); 
#endif

    // 初始化摄像头配置
    err = esp_camera_init(&camera_config);
    if (ESP_OK != err){
        ESP_LOGE(TAG, "Camera Init Failed");
        for(;;){
            vTaskDelay(10);
        }
    }
    sensor_t * s = esp_camera_sensor_get();
    s->set_vflip(s, 1);         //画面翻转
    ESP_LOGI(TAG, "Camera Init OK.");

    // 初始化http服务器
    http_server_init();

    // 创建核心1执行其他功能函数
    xTaskCreatePinnedToCore(AppTask, "AppTask", 1024*8, NULL, 5, NULL, 1);

    // 核心0退出
}

// app任务
static void AppTask(void *pvParameter)
{
    //功能函数从这里开始执行

    // i2c0初始化
    I2c_Master_Init(I2C_MASTER_NUM, I2C_SDA_IO, I2C_SCL_IO);
    ESP_LOGI(TAG, "I2C0 Master init ok.\n");

#if MPU6886_TEST == 1
    float accX = 0;
    float accY = 0;
    float accZ = 0;

    // MPU6886初始化
    MPU6886_Init();
#endif 

    while(true){
#if IP5306_TEST == 1
        //IP5306读电池电量
        printf("IP5306 read ： Power is %d.\n", Ip5306_Check_Power());
#endif

#if MPU6886_TEST == 1
        // MPU6886 陀螺仪测试
        MPU6886_Get_Accel_Data(&accX, &accY, &accZ);
        printf("-----------------------\n");
        printf("%.2f\t%.2f\t%.2f\t\tmg\n", accX*1000, accY*1000, accZ*1000);
#endif 

        printf("app task running...\n");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

// led测试任务
static void Led_Task_Test(void *pvParameter)
{
    // led初始化
    esp_rom_gpio_pad_select_gpio(S003_LED_IO);
    gpio_set_direction(S003_LED_IO, GPIO_MODE_OUTPUT);
    while(1){
        gpio_set_level(S003_LED_IO, 1);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        gpio_set_level(S003_LED_IO, 0);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}