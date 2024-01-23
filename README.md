# ESP32-CAM PSRAM driver test program (the firmware is flashed by factory default)

This routine is implemented based on the ESP-IDF5.3 development framework. Please make sure ESP-IDF5.3 [tutorial address] (https://docs.espressif.com/projects/esp-idf/en/latest) is installed before use. .1/get-started/index.html), the camera used by the program by default is OV5640. Other cameras have not been tested and cannot ensure that the program can run successfully. Please ensure that the module input power is at least 5V2A, otherwise there may be jamming caused by the power supply.

* In AP mode, use a mobile phone or computer to connect to wifi ***ESP32-CAM PSRAM*** Password ***12345678*** Visit ***192.168.4.1/*** in the browser to get the photo stream, visit * **192.168.4.1/jpg*** Get a jpg format picture. The program uses this method by default.
* In STA mode, first modify *WIFI_STA_SSID* and *WIFI_STA_PASS* in the main.c file to the AP ID and password that need to be connected. Of course, first make sure that the AP is visible. After the connection is successful, it can be read from the serial port The IP address obtained by ESP32-CAM PSRAM. Use a mobile phone or computer to connect to the AP, make sure it is on the same network as the ESP32-CAM PSRAM. Use a browser to access ***[ESP32 ip address]/*** to obtain the photo stream, access ***[ESP32 ip address]/jpg** * Get a jpg format picture.
* In smartconfig mode, first open the serial port to view debugging information, use [ESP-TOUCH](https://www.espressif.com/en/support/download/apps?keys=&field_technology_tid%5B%5D=20) Configure the network for the ESP32-CAM PSRAM. After the network configuration is successful, you can view the IP address of the ESP32-CAM PSRAM in the debugging information. The subsequent steps are the same as the above for STA mode.
* The program also integrates the option of testing IP5306 and MPU6886.

**Disclaimer: All programs of this software are for reference only. Our company does not provide any form of guarantee (express or implied) for this program, including but not limited to the correctness, stability, and security of the program. All consequences arising from this procedure must be borne by yourself.**

# ESP32-CAM PSRAM 驱动测试程序(出厂默认刷写该固件)

本例程基于ESP-IDF4.1开发框架实现，使用前请确保安装了ESP-IDF4.1[教程地址](https://docs.espressif.com/projects/esp-idf/zh_CN/release-v4.1/get-started/index.html) ，程序默认使用的摄像头为OV2640，其他摄像头未经测试，不能确保程序能成功运行。请保证模块输入电源至少5V2A否则可能会出现电源引起的卡顿现象。    

* 在AP模式下，使用手机或电脑连接wifi ***ESP32-CAM PSRAM***  密码 ***12345678***  在浏览器访问 ***192.168.4.1/*** 获取照片流，访问 ***192.168.4.1/jpg*** 获取一张jpg格式图片。程序默认使用这种方式。
* 再STA模式下，首先修改main.c文件中的 *WIFI_STA_SSID* 和 *WIFI_STA_PASS* 成需要连接的AP ID和密码，当然首先要确定该AP是可见的，连接成功后可以从串口中读取到ESP32-CAM PSRAM所获取到的IP地址。使用手机或电脑连接AP，确保与ESP32-CAM PSRAM在同一个网络 使用浏览器访问 ***[ESP32 ip地址]/*** 获取照片流， 访问 ***[ESP32 ip地址]/jpg*** 获取一张jpg格式图片。
* 在smartconfig模式下，首先打开串口查看调试信息，使用[ESP-TOUCH](https://www.espressif.com/zh-hans/support/download/apps?keys=&field_technology_tid%5B%5D=20)对ESP32-CAM PSRAM进行配网，配网成功后即可在调试信息中查看ESP32-CAM PSRAM的ip地址，之后步骤同上STA模式一致。  
* 在程序中还集成了可以选择是否进行IP5306和MPU6886的测试。

**免责声明 ：本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。**

