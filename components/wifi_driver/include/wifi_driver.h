/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    wifi_driver.h
 * @version wifi_driver v2.0
 * @brief   基于ESP32 IDF4.1， wifi 配置驱动程序v2.0 接口头文件
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

#ifndef __WIFI_DRIVER_H
#define __WIFI_DRIVER_H

/**
  * @brief  初始化wifi并打开
  * @param  void
  * @retval void
  */
void WIFI_Init(void);

/**
  * @brief  将ESP32 wifi 初始化成AP模式
  * @param  APSSID：softap ID
  * @param  APPASS：softap 密码，小于8位时会忽略
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Softap(const char* APSSID, const char* APPASS);

/**
  * @brief  将ESP32 wifi 初始化成STA模式
  * @param  STASSID sta目标ID
  * @param  STAPASS sta目标密码
  * @retval void
  * @note   通过获取s_wifi_event_group事件组标志位来判断wifi连接状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Init_Sta(const char* STASSID, const char* STAPASS);

/**
  * @brief  ESP32开始SmartConfig
  * @param  void
  * @retval void
  * @note   默认支持ESP TOUCH模式，配网成功后wifi处于sta联网状态
  * @note   这个函数会会自动打开wifi,使用前wifi最好是关闭状态
  */
void WIFI_Smart_Config(void);

/**
  * @brief  ESP32开启wifi
  * @param  void
  * @retval void
  * @note   在模式初始化函数中已经自动开启wifi，此函数非必须调用
  */
void WIFI_On(void);

/**
  * @brief  ESP32关闭wifi
  * @param  void
  * @retval void
  * @note   如果wifista已经连接则先断开
  */
void WIFI_Off(void);

#endif /* __WIFI_DRIVER_H */