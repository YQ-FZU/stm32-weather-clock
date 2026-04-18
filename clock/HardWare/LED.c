#include "stm32f10x.h"                  // Device header
#define LED_PORT	GPIOE
#define LED_PIN		GPIO_Pin_5
void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);//开启RCC时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = LED_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(LED_PORT, &GPIO_InitStructure);
	GPIO_SetBits(LED_PORT, LED_PIN);//初始化后关灯（输出高电平）
}
void LED_OFF(void)
{
	GPIO_SetBits(LED_PORT, LED_PIN);//GPIO输出高电平
}
void LED_ON(void)
{
	GPIO_ResetBits(LED_PORT, LED_PIN);//GPIO输出低电平
}
void BreathingLight(void)//呼吸灯
{
	for (int n = 0; n < 1000; n++)
	{
		for (uint8_t t = 0; t < 5; t++)
		{
			for (int a = 0; a < n; a++)
			{
				LED_ON();		
			}
			for (int a = 1000; a > n; a--)
			{
				LED_OFF();
			}
		}
	}
	for (int n = 1000; n > 0; n--)
	{
		for (uint8_t t = 0; t < 5; t++)
		{
			for (int a = 0; a < n; a++)
			{
				LED_ON();		
			}
			for (int a = 1000; a > n; a--)
			{
				LED_OFF();
			}
		}
	}
}
