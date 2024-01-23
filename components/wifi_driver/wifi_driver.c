/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    wifi_driver.c
 * @version wifi_driver v2.0
 * @brief   基于ESP32 IDF4.1， wifi 配置驱动程序v2.0 接口函数实现
 * @date    2020-12-7
 * @note    
 *          ->分离wifi初始化和模式初始化，使wifi可以动态的切换模式
 *          ->新增wifi打开/关闭接口
 *          ->新增smart config 接口
 *          ->修改wifi模式设置接口，动态修改ID和密码  
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。
 ***********************************************************************************************************/

#include "wifi_driver.h"
#include "esp_smartconfig.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static char TAG[]="WIFI message";           

static int s_retry_num = 0;
static esp_netif_t *netif_ap = NULL;
static esp_netif_t *netif_sta = NULL;
static bool sta_connect = false;

/*-------------AP-------------*/
#define ESP_WIFI_CHANNEL_AP         1                   //AP wifi 通道
#define MAX_STA_CONN_AP             5                   //AP 最大连接数量

/*-------------STA------------*/
#define DEFAULT_SCAN_LIST_SIZE      15                  //最大扫描个数

#define WIFI_CONNECTED_BIT          BIT0             
#define WIFI_FAIL_BIT               BIT1                
#define WIFI_SCAN_DONE_BIT          BIT2                
#define WIFI_ESPTOUCH_DONE_BIT      BIT3              

EventGroupHandle_t  s_wifi_event_group;    

typedef struct {
    char SSID[33];
    int8_t RSSI;
}wifi_scan_list_t;  

/**
  * @brief  wifi事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID，查看esp_wifi_types.h
  * @param  event_data:事件数据
  * @retval void
  * @note   根据wifi的连接事件设置事件组标志位
  */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {              
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        //ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
        //         MAC2STR(event->mac), event->aid);
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);    
    }else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {      
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        //ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
        //         MAC2STR(event->mac), event->aid);
        xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT); 
    }else if (event_id == WIFI_EVENT_STA_START) {             
        esp_wifi_connect();
    }else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {      
        if (s_retry_num < 5) {                            
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {                                                       
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);   
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }else if (event_id == WIFI_EVENT_SCAN_DONE){
        xEventGroupSetBits(s_wifi_event_group, WIFI_SCAN_DONE_BIT);    
    }
}

/**
  * @brief  ip事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID，查看esp_wifi_types.h
  * @param  event_data:事件数据
  * @retval void
  * @note   根据wifi的连接事件设置事件组标志位
  */
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if (event_id == IP_EVENT_STA_GOT_IP) {            
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        sta_connect = true;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);    
    }
}

/**
  * @brief  smart config事件处理回调函数
  * @param  arg:传入参数，此处为NULL
  * @param  event_base:指向公开事件的子系统的唯一指针
  * @param  event_id:事件ID
  * @param  event_data:事件数据
  * @retval void
  * @note   打印sc的过程信息
  */
static void sc_event_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    if(event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE){                   
        ESP_LOGI(TAG, "SC  Scan done");
    }else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL){          
        ESP_LOGI(TAG, "SC  Found channel");
    }else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD){       
        ESP_LOGI(TAG, "SC  Got SSID and password");
        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = { 0 };
        uint8_t password[65] = { 0 };

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true) {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        ESP_LOGI(TAG, "SSID:%s", ssid);
        ESP_LOGI(TAG, "PASSWORD:%s", password);

        ESP_ERROR_CHECK(esp_wifi_disconnect());                                
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));     
        ESP_ERROR_CHECK(esp_wifi_connect());                                      
    }else if(event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE){        
        xEventGroupSetBits(s_wifi_event_group, WIFI_ESPTOUCH_DONE_BIT);
    }
}

/**
  * @brief  初始化wifi并打开
  * @param  void
  * @retval void
  */
void WIFI_Init(void)
{
    s_wifi_event_group = xEventGroupCreate();              
    ESP_ERROR_CHECK(esp_netif_init());                         
    ESP_ERROR_CHECK(esp_event_loop_create_default() );     
    netif_ap = esp_netif_create_default_wifi_ap();         
    assert(netif_ap);
    netif_sta = esp_netif_create_default_wifi_sta();      
    assert(netif_sta);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();  
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                  
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));      
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));      
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL) );   
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));    
    sta_connect = false;
}

/**
  * @brief  将ESP32 wifi 初始化成AP模式
  * @param  APSSID：softap ID
  * @param  APPASS：softap 密码，小于8位时会忽略
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Softap(const char* APSSID, const char* APPASS)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_config_t wifi_config = {                      
        .ap = {
            .ssid = "",                                     
            .ssid_len = 0,                                        
            .channel = ESP_WIFI_CHANNEL_AP,                  
            .password = "",                                  
            .max_connection = MAX_STA_CONN_AP,                
            .authmode = WIFI_AUTH_WPA_WPA2_PSK              
        },
    };
    strlcpy((char*)wifi_config.ap.ssid, APSSID, sizeof(wifi_config.ap.ssid));
    if(strlen(APPASS) < 8){                          
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        ESP_LOGI(TAG, "password less than 8, The pattern is converted to auth open.");
    }else{
        strlcpy((char*)wifi_config.ap.password, APPASS, sizeof(wifi_config.ap.password));
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));                              
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));                           
    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d", 
             APSSID, APPASS, ESP_WIFI_CHANNEL_AP);
}

/**
  * @brief  将ESP32 wifi 初始化成STA模式
  * @param  STASSID sta目标ID
  * @param  STAPASS sta目标密码
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Sta(const char* STASSID, const char* STAPASS)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    wifi_config_t wifi_config = {                  
        .sta = {
            .ssid = "",                                  
            .password = "",                                   
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,          
            .pmf_cfg = {                                       
                .capable = true,
                .required = false
            },
        },
    };
    strlcpy((char*)wifi_config.sta.ssid, STASSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char*)wifi_config.sta.password, STAPASS, sizeof(wifi_config.sta.password));

    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                      
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));    
    ESP_ERROR_CHECK(esp_wifi_connect());
    ESP_LOGI(TAG, "wifi init sta finished.");

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,              
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, 
                                           pdFALSE, 
                                           pdFALSE, 
                                           portMAX_DELAY);
    if (bits & WIFI_CONNECTED_BIT){                                     
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 STASSID, STAPASS);
        sta_connect = true;
    }else if(bits & WIFI_FAIL_BIT){                                     
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 STASSID, STAPASS);
        sta_connect = false;
    }else{                                                             
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        sta_connect = false;
    }
}

/**
  * @brief  ESP32开始SmartConfig
  * @param  void
  * @retval void
  * @note   默认支持ESP TOUCH模式，配网成功后wifi处于sta联网状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Smart_Config(void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
    EventBits_t uxBits;
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));                    
    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));            
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&sc_cfg));                        
    while(1){                                                               
        uxBits = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);    
        if(uxBits & WIFI_CONNECTED_BIT){ 
            ESP_LOGI(TAG, "WiFi Connected to ap");
            sta_connect = true;
        }
        if(uxBits & WIFI_ESPTOUCH_DONE_BIT){ 
            ESP_LOGI(TAG, "smartconfig over");
            esp_smartconfig_stop();
            return;
        }
    }
}

/**
  * @brief  ESP32开启wifi
  * @param  void
  * @retval void
  * @note   在模式初始化函数中已经自动开启wifi，此函数非必须调用
  */
void WIFI_On(void)
{
    ESP_ERROR_CHECK(esp_wifi_start());
}

/**
  * @brief  ESP32关闭wifi
  * @param  void
  * @retval void
  * @note   如果wifista已经连接则先断开
  */
void WIFI_Off(void)
{   
    if(true == sta_connect){
        s_retry_num = 5;
        ESP_ERROR_CHECK(esp_wifi_disconnect());
        xEventGroupWaitBits(s_wifi_event_group,
                            WIFI_FAIL_BIT, 
                            pdFALSE, 
                            pdFALSE, 
                            portMAX_DELAY);
        xEventGroupClearBits(s_wifi_event_group, WIFI_FAIL_BIT);
        sta_connect = false;
        ESP_LOGI(TAG, "sta disconnect\n");
    }

    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
}
