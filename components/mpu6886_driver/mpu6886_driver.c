/***********************************************************************************************************
 * @warning 
 *          本软件的所有程序仅供参考学习，本公司对此程序不提供任意形式（任何明示或暗示）的担保，包括但不限于程序的正
 *          确性、稳定性、安全性，因使用此程序产生的一切后果需自我承担。 
 ***********************************************************************************************************/

#include "mpu6886_driver.h"
#include "../i2c_master/include/i2c_master.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c_master.h"

enum Gscale Gyscale = GFS_2000DPS;
enum Ascale Acscale = AFS_8G;

static const char *TAG = "MPU6886";

static i2c_master_dev_handle_t dev_handle;


/**
  * @brief  MPU6886读n个字节
  * @param  reg_Addr：待读出的寄存器地址
  * @param  len：长度
  * @param  read_Buffer:读出数据的存放地址
  * @retval 参考esp_err_t
  */
static esp_err_t MPU6886I2C_Read_NBytes(uint8_t reg_Addr, uint8_t len, uint8_t *read_Buffer)
{
    return i2c_master_transmit_receive(
              dev_handle,
              (const uint8_t *)&reg_Addr,
              1,
              read_Buffer,
              len,
              -1
          );
}

/**
  * @brief  MPU6886写n个字节
  * @param  reg_Addr：待写入的寄存器地址
  * @param  len：长度
  * @param  write_Buffer:待写入的数据指针
  * @retval 参考esp_err_t
  */
static esp_err_t MPU6886I2C_Write_NBytes(uint8_t reg_Addr, uint8_t len, uint8_t *write_Buffer)
{
    return i2c_master_transmit(
      dev_handle,
      write_Buffer,
      len,
      -1
    );
}

/**
  * @brief  MPU6886初始化
  * @param  void
  * @retval 成功0，失败1
  */
int32_t MPU6886_Init(void)
{
    uint8_t tempdata[1];
    uint8_t regdata;

    MPU6886I2C_Read_NBytes(MPU6886_WHOAMI, 1, tempdata);
    if (tempdata[0] != 0x19)
        return -1;
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x00;
    MPU6886I2C_Write_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);       // PWR_MGMT_1(0x6b)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = (0x01 << 7);
    MPU6886I2C_Write_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);       // PWR_MGMT_1(0x6b)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = (0x01 << 0);
    MPU6886I2C_Write_NBytes(MPU6886_PWR_MGMT_1, 1, &regdata);       // PWR_MGMT_1(0x6b)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x10;
    MPU6886I2C_Write_NBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);     // ACCEL_CONFIG(0x1c) : +-8G
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x18;
    MPU6886I2C_Write_NBytes(MPU6886_GYRO_CONFIG, 1, &regdata);      // GYRO_CONFIG(0x1b) : +-2000dps
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x01;
    MPU6886I2C_Write_NBytes(MPU6886_CONFIG, 1, &regdata);           // CONFIG(0x1a)    
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x05;
    MPU6886I2C_Write_NBytes(MPU6886_SMPLRT_DIV, 1, &regdata);       // SMPLRT_DIV(0x19)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x00;
    MPU6886I2C_Write_NBytes(MPU6886_INT_ENABLE, 1, &regdata);       // INT_ENABLE(0x38)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x00;
    MPU6886I2C_Write_NBytes(MPU6886_ACCEL_CONFIG2, 1, &regdata);    // ACCEL_CONFIG 2(0x1d)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x00;
    MPU6886I2C_Write_NBytes(MPU6886_USER_CTRL, 1, &regdata);        // USER_CTRL(0x6a)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x00;
    MPU6886I2C_Write_NBytes(MPU6886_FIFO_EN, 1, &regdata);          // FIFO_EN(0x23)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x22;
    MPU6886I2C_Write_NBytes(MPU6886_INT_PIN_CFG, 1, &regdata);      // INT_PIN_CFG(0x37)
    vTaskDelay(10 / portTICK_PERIOD_MS);

    regdata = 0x01;
    MPU6886I2C_Write_NBytes(MPU6886_INT_ENABLE, 1, &regdata);       // INT_ENABLE(0x38)

    vTaskDelay(100 / portTICK_PERIOD_MS);
    MPU6886_Get_Gres();
    MPU6886_Get_Ares();
    return 0;
}

void MPU6886_Get_Accel_Adc(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];
    MPU6886I2C_Read_NBytes(MPU6886_ACCEL_XOUT_H, 6, buf);

    *ax = ((int16_t)buf[0] << 8) | buf[1];
    *ay = ((int16_t)buf[2] << 8) | buf[3];
    *az = ((int16_t)buf[4] << 8) | buf[5];
}

void MPU6886_Get_Gyro_Adc(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[6];
    MPU6886I2C_Read_NBytes(MPU6886_GYRO_XOUT_H, 6, buf);

    *gx = ((uint16_t)buf[0] << 8) | buf[1];
    *gy = ((uint16_t)buf[2] << 8) | buf[3];
    *gz = ((uint16_t)buf[4] << 8) | buf[5];
}

void MPU6886_Get_Temp_Adc(int16_t *t)
{
    uint8_t buf[2];
    MPU6886I2C_Read_NBytes(MPU6886_TEMP_OUT_H, 2, buf);

    *t = ((uint16_t)buf[0] << 8) | buf[1];
}

void MPU6886_Get_Gres(void)
{
    switch (Gyscale){
        // Possible gyro scales (and their register bit settings) are:
    case GFS_250DPS:
        gRes = 250.0 / 32768.0;
        break;
    case GFS_500DPS:
        gRes = 500.0 / 32768.0;
        break;
    case GFS_1000DPS:
        gRes = 1000.0 / 32768.0;
        break;
    case GFS_2000DPS:
        gRes = 2000.0 / 32768.0;
        break;
    }
}

void MPU6886_Get_Ares(void)
{
    switch (Acscale){
        // Possible accelerometer scales (and their register bit settings) are:
        // 2 Gs (00), 4 Gs (01), 8 Gs (10), and 16 Gs  (11).
        // Here's a bit of an algorith to calculate DPS/(ADC tick) based on that 2-bit value:
    case AFS_2G:
        aRes = 2.0 / 32768.0;
        break;
    case AFS_4G:
        aRes = 4.0 / 32768.0;
        break;
    case AFS_8G:
        aRes = 8.0 / 32768.0;
        break;
    case AFS_16G:
        aRes = 16.0 / 32768.0;
        break;
    }
}

void MPU6886_Set_FIFO_Enable(bool enableflag)
{
    uint8_t regdata = 0;
    if(enableflag){
        regdata = 0x0c;
        MPU6886I2C_Write_NBytes(MPU6886_FIFO_ENABLE, 1, &regdata);
        regdata = 0x40;
        MPU6886I2C_Write_NBytes(MPU6886_USER_CTRL, 1, &regdata);
    }else{
        regdata = 0x00;
        MPU6886I2C_Write_NBytes(MPU6886_FIFO_ENABLE, 1, &regdata);
        regdata = 0x00;
        MPU6886I2C_Write_NBytes(MPU6886_USER_CTRL, 1, &regdata);
    }
}

uint8_t MPU6886_Read_FIFO(void)
{
    uint8_t ReData = 0;
    MPU6886I2C_Read_NBytes(MPU6886_FIFO_R_W, 1, &ReData);
    return ReData;
}

void MPU6886_Read_FIFO_Buff(uint8_t *DataBuff, uint16_t Length)
{
    MPU6886I2C_Read_NBytes(MPU6886_FIFO_R_W, Length, DataBuff);
}

uint16_t MPU6886_Read_FIFO_Count(void)
{
    uint8_t Buff[2];
    uint16_t ReData = 0;
    MPU6886I2C_Read_NBytes(MPU6886_FIFO_CONUTH, 2, Buff);
    ReData = Buff[0];
    ReData <<= 8;
    ReData |= Buff[1];
    return ReData;
}

void MPU6886_Set_Gyro_Fsr(enum Gscale scale)
{
    //设置陀螺仪满量程范围
    unsigned char regdata;
    regdata = (scale << 3);
    MPU6886I2C_Write_NBytes(MPU6886_GYRO_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);

    Gyscale = scale;
    MPU6886_Get_Gres();
}

void MPU6886_Set_Accel_Fsr(enum Ascale scale)
{
    //设置加速度满量程范围
    unsigned char regdata;
    regdata = (scale << 3);
    MPU6886I2C_Write_NBytes(MPU6886_ACCEL_CONFIG, 1, &regdata);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    Acscale = scale;
    MPU6886_Get_Ares();
}

void MPU6886_Get_Accel_Data(float *ax, float *ay, float *az)
{
    int16_t accX = 0;
    int16_t accY = 0;
    int16_t accZ = 0;
    MPU6886_Get_Accel_Adc(&accX, &accY, &accZ);

    *ax = (float)accX * aRes;
    *ay = (float)accY * aRes;
    *az = (float)accZ * aRes;
}

void MPU6886_Get_Gyro_Data(float *gx, float *gy, float *gz)
{
    int16_t gyroX = 0;
    int16_t gyroY = 0;
    int16_t gyroZ = 0;
    MPU6886_Get_Gyro_Adc(&gyroX, &gyroY, &gyroZ);

    *gx = (float)gyroX * gRes;
    *gy = (float)gyroY * gRes;
    *gz = (float)gyroZ * gRes;
}

void MPU6886_Get_Temp_Data(float *t)
{
    int16_t temp = 0;
    MPU6886_Get_Temp_Adc(&temp);

    *t = (float)temp / 326.8 + 25.0;
}
