#include "MPU6050.h"
#include "stm32f10x.h"
#include "MPU6050_Reg.h"
#include "my_iic.h"
#include "./SYSTEM/delay/delay.h"
#define MPU6050_ADDRESS		0xD0		//MPU6050的I2C从机地址
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)//指定地址写
{
    my_iic_start();
	my_iic_send_byte(MPU6050_ADDRESS | 0x00);//发送从机地址，读写位为0，表示即将写入
    my_iic_receive_ack();
    my_iic_send_byte(RegAddress);//发送寄存器地址
    my_iic_receive_ack();
    my_iic_send_byte(Data);//发送要写入寄存器的数据
    my_iic_receive_ack();
    my_iic_stop();
}
uint8_t MPU6050_ReadReg(uint8_t RegAddress)//指定地址读
{
    uint8_t Data;
    //指定地址写
    my_iic_start();
    my_iic_send_byte(MPU6050_ADDRESS | 0x00);//发送从机地址，读写位为0(AD=0)，表示即将写入
    my_iic_receive_ack();
    my_iic_send_byte(RegAddress);//发送寄存器地址
    my_iic_receive_ack();
    //当前地址读
    my_iic_start();
    my_iic_send_byte(MPU6050_ADDRESS | 0x01);//发送从机地址，读写位为1，表示即将读取
    my_iic_receive_ack();
    Data = my_iic_receive_byte();//接收指定寄存器的数据
    my_iic_send_ack(1);//发送应答，给从机非应答，终止从机的数据输出
    my_iic_stop();
    return Data;
}
void MPU6050_Init(void)
{
    my_iic_init();//先初始化底层的I2C
    /*MPU6050寄存器初始化，需要对照MPU6050手册的寄存器描述配置，此处仅配置了部分重要的寄存器*/
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);//电源管理寄存器1，取消睡眠模式，选择时钟源为X轴陀螺仪
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);		//电源管理寄存器2，保持默认值0，所有轴均不待机
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x09);		//采样率分频寄存器，配置采样率
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);			//配置寄存器，配置DLPF
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);	//陀螺仪配置寄存器，选择满量程为±2000°/s
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);	//加速度计配置寄存器，选择满量程为±16g
}
 uint8_t MPU6050_GetID(void)//MPU6050获取ID号
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);		//返回WHO_AM_I寄存器的值
}

/**
  * 函    数：MPU6050获取数据
  * 参    数：AccX AccY AccZ 加速度计X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 参    数：GyroX GyroY GyroZ 陀螺仪X、Y、Z轴的数据，使用输出参数的形式返回，范围：-32768~32767
  * 返 回 值：无
  */
 MPU6050_Data  MPU6050_GetData(void)//获取MPU6050各项参数数据
 {
    MPU6050_Data MPU_Data;
    uint8_t DataH, DataL;								//定义数据高8位和低8位的变量
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);		//读取加速度计X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);		//读取加速度计X轴的低8位数据
	MPU_Data.AccX = (int16_t)((DataH << 8) | DataL);			
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);		//读取加速度计Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);		//读取加速度计Y轴的低8位数据
	MPU_Data.AccY = (int16_t)((DataH << 8) | DataL);						
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);		//读取加速度计Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);		//读取加速度计Z轴的低8位数据
	MPU_Data.AccZ = (int16_t)((DataH << 8) | DataL);						
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);		//读取陀螺仪X轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);		//读取陀螺仪X轴的低8位数据
	MPU_Data.GyroX = (int16_t)((DataH << 8) | DataL) ;				
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);		//读取陀螺仪Y轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);		//读取陀螺仪Y轴的低8位数据
	MPU_Data.GyroY = (int16_t)((DataH << 8) | DataL) ;				
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);		//读取陀螺仪Z轴的高8位数据
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);		//读取陀螺仪Z轴的低8位数据
	MPU_Data.GyroZ = (int16_t)((DataH << 8) | DataL) ;	
    return MPU_Data;			
 }
 void mpu6050_get_temper(float *temper)
 {
	uint8_t DataH, DataL;
	DataH = MPU6050_ReadReg(MPU6050_TEMP_OUT_H);		
	DataL = MPU6050_ReadReg(MPU6050_TEMP_OUT_L);
	*temper = (int16_t)((DataH << 8) | DataL) / 340.0f + 36.53f;
 }
 
 