#include "stm32f10x.h"  
#include "esp_usart.h"
#include <string.h>
_Bool volatile usart_send_isbusy;
static usart_receive_callback_t usart_receive_callback;//定义接收回调函数名
static esp_usart_receive_callback_t esp_usart_receive_callback;
void usart_goio_init(void)
{
	//PA9 TX
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,& GPIO_InitStructure);
	//PA10 RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,& GPIO_InitStructure);
}
void uart_nvic_init(void)
{
	//配置NVIC，采用中断查询方式接收数据
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);
	
	NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);
}
void usart_lowlevel_init(void)
{
	//配置USART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	//使能串口接收数据
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	//USART使能DMA
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
    /*USART使能*/
	USART_Cmd(USART1, ENABLE);	
}
void Usart_Init(void)
{
	//开启usart模块时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	usart_goio_init();
	uart_nvic_init();
	usart_lowlevel_init();						
}
//串口发送字节
void Usart_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) != SET);//等待发送完成
}
//串口发送数组
void Usart_SendArray(uint8_t *Array, uint16_t length)
{
	for (uint16_t i = 0; i < length; i++)
	{
		Usart_SendByte(Array[i]);
	}
}
//串口发送字符串
void Usart_SendString(const char *string)
{
	for(uint8_t i=0;string[i]!='\0';i++)
    {
        Usart_SendByte(string[i]);
    }
}
//用于注册串口接收回调函数
void Usart_Receive_Register(usart_receive_callback_t callback)
{
	usart_receive_callback = callback;
}
//采用DMA发送数据
void Usart_DMA_Write(uint8_t *Data, uint16_t DataSize)
{
	//while (usart_send_isbusy == 1);									//等待DMA一轮发送完成
	//配置DMA用于发送数据
	DMA_DeInit(DMA1_Channel4);										//初始化DMA
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_BufferSize = DataSize;						//转运的数据大小（转运次数）
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralDST;					//数据传输方向，选择存储器到外设寄存器
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;						//存储器到存储器
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)Data;				//存储器基地址
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;	//存储器数据宽度，选择字节
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;			//存储器地址自增，选择使能
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;						//模式，选择正常模式
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;	//外设基地址
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;				//优先级，选择中等
    DMA_Init(DMA1_Channel4, & DMA_InitStruct);						//USART1选择通道4
	//使能串口DMA发送
	DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE);
	usart_send_isbusy = 1;
	DMA_Cmd(DMA1_Channel4, ENABLE);
	while (usart_send_isbusy == 1);									//等待DMA一轮发送完成
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t Rx_Data = USART_ReceiveData(USART1);
		if (usart_receive_callback)
		{
			usart_receive_callback(Rx_Data);	//调用回调函数
		}
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}
void DMA1_Channel4_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC4) == SET)	//如果DMA转运完成则触发中断
	{
		usart_send_isbusy = 0;
		DMA_ClearITPendingBit(DMA1_IT_TC4);
	}
}
//usart2作为单片机与esp32直接通信
void esp_usart_init(void)
{
	//开启usart模块时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	//PA2 TX
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,& GPIO_InitStructure);
	//PA3 RX
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,& GPIO_InitStructure);
	
	//配置USART
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &USART_InitStructure);
	//使能串口接收数据
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	
	
	//配置NVIC，采用中断查询方式接收数据
    NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStruct);
	
    /*USART使能*/
	USART_Cmd(USART2, ENABLE);	
}

void esp_usart_send_byte(uint8_t Byte)
{
	USART_SendData(USART2, Byte);//将字节数据写入数据寄存器，写入后USART自动生成时序波形
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET);//等待发送完成
}

void esp_usart_send_array(const uint8_t *Array, const uint32_t Length)
{
	for (uint16_t i = 0; i < Length; i++)
	{
		esp_usart_send_byte(Array[i]);
	}
}
void esp_usart_send_string(const char *string)
{
	uint16_t len = strlen(string);
     esp_usart_send_array((uint8_t *)string, len);
}

void esp_usart_receive_register(usart_receive_callback_t esp_callback)
{
	esp_usart_receive_callback = esp_callback;
}

void USART2_IRQHandler(void)
{
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		uint8_t Rx_Data = USART_ReceiveData(USART2);
		if (esp_usart_receive_callback)
		{
			esp_usart_receive_callback(Rx_Data);	//调用回调函数
		}
		
	}
}
