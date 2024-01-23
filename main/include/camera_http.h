/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    camera_http.c
 * @version camera_http v1.0
 * @brief   基于ESP32 IDF4.1， 摄像头http服务器处理接口头文件。
 * @date    2021-1-30
 * @note    
 *          ->提供一个http服务器初始化接口，初始化一个/jpg URL 和 / URL
 * @note    
 *          使用该文件接口前应确保esp32以及连接或者创键AP，或以其他方式连接网络
 *          wifi AP 模式下 访问 192.168.4.1/ 获取照片流， 访问192.168.4.1/jpg 获取一张jpg格式图片
 *          wifi STA 模式下 访问 [ESP32 ip地址]/ 获取照片流， 访问[ESP32 ip地址]/jpg 获取一张jpg格式图片
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。 
 ***********************************************************************************************************/

#ifndef __CAMERA_HTTP_H__
#define __CAMERA_HTTP_H__

/**
  * @brief  初始化http服务器并创建“/jpg”的URL
  * @param  void
  * @retval void
  * @note   使用该接口前确保esp32已经连接或创建网络
  */
void http_server_init(void);

#endif /* __CAMERA_HTTP_H__ */