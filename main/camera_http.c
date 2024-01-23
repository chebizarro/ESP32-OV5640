/***********************************************************************************************************
 * 优仕德电子
 * ESP32 ESP-IDF4 DriverLib
 * @file    camera_http.c
 * @version camera_http v1.0
 * @brief   基于ESP32 IDF4.1， 摄像头http服务器处理接口实现文件。
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

#include "camera_http.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "wifi_driver.h"
#include "esp_camera.h"

static char TAG[] = "Camera http";

#define PART_BOUNDARY "123456789000000000000987654321"
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t http_send_jpg_handler(httpd_req_t *req);
static esp_err_t jpg_stream_httpd_handler(httpd_req_t *req);

/**
  * @brief  初始化http服务器
  * @param  void
  * @retval void
  * @note   使用该接口前确保esp32已经连接或创建网络
  */
void http_server_init(void)
{
    httpd_handle_t server;

    httpd_uri_t jpeg_stream_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = jpg_stream_httpd_handler,
        .user_ctx = NULL
    };

    httpd_uri_t jpeg_uri = {
        .uri = "/jpg",
        .method = HTTP_GET,
        .handler = http_send_jpg_handler,
        .user_ctx = NULL
    };

    httpd_config_t http_options = HTTPD_DEFAULT_CONFIG();

    ESP_ERROR_CHECK(httpd_start(&server, &http_options));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &jpeg_stream_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &jpeg_uri));
}

/**
  * @brief  http / URL(获取照片流)处理函数
  * @param  req ：HTTP请求数据结构
  * @retval 参考esp_err
  * @note   使用该接口前确保esp32已经连接或创建网络
  * @note   进入后程序将会在里面循环
  */
static esp_err_t jpg_stream_httpd_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    struct timeval _timestamp;
    esp_err_t res = ESP_OK;
    size_t _jpg_buf_len = 0;
    uint8_t *_jpg_buf = NULL;
    char *part_buf[128];

    static int64_t last_frame = 0;
    if(!last_frame){
        last_frame = esp_timer_get_time();
    }

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if(res != ESP_OK){
        return res;
    }

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true){
        fb = esp_camera_fb_get();
        if (!fb){
            ESP_LOGE(TAG, "Camera capture failed");
            res = ESP_FAIL;
        }else{
            _timestamp.tv_sec = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
            if(fb->format != PIXFORMAT_JPEG){
                bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
                esp_camera_fb_return(fb);
                fb = NULL;
                if (!jpeg_converted){
                    ESP_LOGE(TAG, "JPEG compression failed");
                    res = ESP_FAIL;
                }
            }else{
                _jpg_buf_len = fb->len;
                _jpg_buf = fb->buf;
            }
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        }
        if(res == ESP_OK){
            size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
            res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
        }
        if(res == ESP_OK){
            res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
        }
        if(fb){
            esp_camera_fb_return(fb);
            fb = NULL;
            _jpg_buf = NULL;
        }else if(_jpg_buf){
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if(res != ESP_OK){
            break;
        }
        int64_t fr_end = esp_timer_get_time();
        int64_t frame_time = fr_end - last_frame;
        last_frame = fr_end;
        frame_time /= 1000;
        //ESP_LOGI(TAG, "MJPG: %uKB %ums (%.1ffps)",(uint32_t)(_jpg_buf_len/1024),
        //        (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
    }

    last_frame = 0;
    return res;
}

/**
  * @brief  http /jpg URL（获取一张JPG格式照片）处理函数
  * @param  req ：HTTP请求数据结构
  * @retval 参考esp_err
  * @note   使用该接口前确保esp32已经连接或创建网络
  */
static esp_err_t http_send_jpg_handler(httpd_req_t *req)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    int64_t fr_start = esp_timer_get_time();
    char ts[32];

    fb = esp_camera_fb_get();
    if(!fb){
        ESP_LOGE(TAG, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, "image/jpeg");
    httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

    snprintf(ts, 32, "%ld.%06ld", (long int)fb->timestamp.tv_sec, fb->timestamp.tv_usec);
    httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

    size_t fb_len = 0;
    if(fb->format == PIXFORMAT_JPEG){
        fb_len = fb->len;
        res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
    }
    esp_camera_fb_return(fb);

    //int64_t fr_end = esp_timer_get_time();
    //ESP_LOGI(TAG, "JPG: %uB %ums", (uint32_t)(fb_len), (uint32_t)((fr_end - fr_start) / 1000));

    return res;
}
