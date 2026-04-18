#include "stm32f10x.h"                  // Device header
#include "./SYSTEM/delay/delay.h"
#define IIC_PORT	GPIOA
#define IIC_SCL		GPIO_Pin_6
#define IIC_SDA		GPIO_Pin_7
void my_iic_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Pin = IIC_SCL | IIC_SDA;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(IIC_PORT, &GPIO_InitStructure);
	GPIO_SetBits(IIC_PORT, IIC_SCL | IIC_SDA);
}
void my_iic_w_scl(uint8_t bit_value)
{
	GPIO_WriteBit(IIC_PORT, IIC_SCL, (BitAction)bit_value);
	delay_us(10);
}
void my_iic_w_sda(uint8_t bit_value)
{
	GPIO_WriteBit(IIC_PORT, IIC_SDA, (BitAction)bit_value);
	delay_us(10);
}
uint8_t my_iic_r_scl(void)
{
	uint8_t bit_value = GPIO_ReadInputDataBit(IIC_PORT, IIC_SCL);
	delay_us(10);
	return bit_value;
}
uint8_t my_iic_r_sda(void)
{
	uint8_t bit_value = GPIO_ReadInputDataBit(IIC_PORT, IIC_SDA);
	delay_us(10);
	return bit_value;
}
void my_iic_start(void)
{
	my_iic_w_sda(1);		//优先拉高sda,由于scl在高电平期间不允许sda变化
	my_iic_w_scl(1);
	my_iic_w_sda(0);
	my_iic_w_scl(0);		//把scl置于0方便读取数据
}
void my_iic_stop(void)
{
	my_iic_w_sda(0);		//优先拉高sda
	my_iic_w_scl(1);
	my_iic_w_sda(1);
}
void my_iic_send_byte(uint8_t byte)
{
	for (uint8_t i = 0; i < 8; i++)
	{
		my_iic_w_sda(byte & (0x80 >> i));
		my_iic_w_scl(1);
		my_iic_w_scl(0);
	}
}
uint8_t my_iic_receive_byte(void)
{
	uint8_t rec_byte = 0x00;
	my_iic_w_scl(0);		//必须先拉低scl再释放sda
	my_iic_w_sda(1);		//释放sda给从机
	for (uint8_t i = 0; i < 8; i++)
	{
		my_iic_w_scl(1);
		if (my_iic_r_sda())
		{
			rec_byte |= (0x80 >> i);
		}
		my_iic_w_scl(0);
	}
	return rec_byte;
}
void my_iic_send_ack(uint8_t ack)
{
	my_iic_w_sda((BitAction) ack);
	my_iic_w_scl(1);
	my_iic_w_scl(0);
}
uint8_t my_iic_receive_ack(void)
{
	uint8_t ack;
	my_iic_w_sda(1);
	my_iic_w_scl(1);
	ack = my_iic_r_sda();
	my_iic_w_scl(0);
	return ack;
}
