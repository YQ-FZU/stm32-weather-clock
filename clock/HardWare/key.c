#include "stm32f10x.h"                  // Device header
#include "./SYSTEM/delay/delay.h"
#include "LED.h"
#define KEY_POART			GPIOE
#define KEY0				GPIO_Pin_4
#define KEY1				GPIO_Pin_3
#define KEY_EXIT_PORT_SRC	GPIO_PortSourceGPIOA	
#define KEY_EXIT_PORT		GPIOA
#define KEY_EXIT_PIN		GPIO_Pin_0
#define KEY_EXTI_LINE		EXTI_Line0
void Key_Init(void)//初始化按键
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//使用上拉输入，悬空时默认高电平
	GPIO_InitStructure.GPIO_Pin = KEY0 | KEY1;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(KEY_POART, &GPIO_InitStructure);
	
}
void Key_Exit_Init(void)
{
	//开启时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);//开启GPIO时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);//开启AFIO时钟
	
	//配置GPIO
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;//下拉输入，悬空时默认为低电平
	GPIO_InitStructure.GPIO_Pin = KEY_EXIT_PIN;
	GPIO_InitStructure.GPIO_Speed  = GPIO_Speed_50MHz;
	GPIO_Init(KEY_EXIT_PORT, &GPIO_InitStructure);
	
	//配置AFIO中断引脚选择
	GPIO_EXTILineConfig(KEY_EXIT_PORT_SRC, KEY_EXIT_PIN);
	
	//配置EXTI
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = KEY_EXTI_LINE;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(& EXTI_InitStructure);
	
	//NVIV中断分组
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(& NVIC_InitStructure);
	
}
uint8_t KeyGetNum(void)
{
	uint8_t KeyNum = 200;
	if(GPIO_ReadInputDataBit(KEY_POART, KEY0) == 0)//按键0按下
	{
		delay_ms(20);
		while(GPIO_ReadInputDataBit(KEY_POART, KEY0) == 0);//判断松开
		delay_ms(20);
		KeyNum = 0;
	}
	if(GPIO_ReadInputDataBit(KEY_POART, KEY1) == 0)//按键1按下
	{
		delay_ms(20);
		while(GPIO_ReadInputDataBit(KEY_POART, KEY1) == 0);//判断松开
		delay_ms(20);
		KeyNum = 1;
	}
	return KeyNum;
}
//中断服务函数
void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(KEY_EXTI_LINE) == SET)
	{
		LED_ON();
		EXTI_ClearITPendingBit(KEY_EXTI_LINE);
	}
}
