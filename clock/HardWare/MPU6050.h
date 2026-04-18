#ifndef __MPU6050_H_
#define __MPU6050_H_
#include "stm32f10x.h"
 typedef struct//定义用于返回MPU6050数据的结构体
{
    int16_t AccX, AccY, AccZ, GyroX, GyroY, GyroZ;
}MPU6050_Data;
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data);//指定地址写
uint8_t MPU6050_ReadReg(uint8_t RegAddress);//指定地址读
void MPU6050_Init(void);//初始化MPU6050
MPU6050_Data MPU6050_GetData(void);//获取MPU6050各项参数数据
uint8_t MPU6050_GetID(void);//MPU6050获取ID号
void mpu6050_get_temper(float *temper);

#endif
