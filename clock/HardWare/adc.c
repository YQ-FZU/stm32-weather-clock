#include "stm32f10x.h"                  // Device header
#include <math.h>  // 用于自然对数ln()函数
#define AD_RESOLUTION  4096.0f   // AD分辨率（12位=4096，8位=256，10位=1024）
#define V_REF   3.3f      // 参考电压（单片机/AD模块供电电压，如3.3V或5V）
#define R0             10000.0f  // 模块内串联固定电阻（常见10KΩ，需确认）
#define R25            10000.0f  // NTC热敏电阻25℃标准阻值（如10KΩ，看规格书）
#define B_VALUE        3950.0f   // NTC的B值（如3950，需从规格书获取）
#define T25            298.15f   // 25℃对应的开尔文温度（25+273.15）
#define adc_buffer_size 	128
uint16_t adc_buffer[adc_buffer_size];
_Bool volatile adc_is_busy;
void adc_gpio_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}
void adc_dma_init(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_DeInit(DMA1_Channel1);										//初始化DMA
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.DMA_BufferSize = adc_buffer_size;						
    DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;					
    DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;						
    DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;				
    DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;	//adc1的规则组在低16位，也就是半字
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralInc = DISABLE;
    DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;						
    DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;	
    DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;				
    DMA_Init(DMA1_Channel1, & DMA_InitStruct);		
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
}
void adc_nvic_init()
{
	NVIC_InitTypeDef NVIC_InitStruct;
    NVIC_InitStruct.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_Init(&NVIC_InitStruct);
}
void adc_lowlevel_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 		//设置时钟6分频为12mhz
	ADC_InitTypeDef ADC_InitSturcture;
	ADC_InitSturcture.ADC_ContinuousConvMode = ENABLE;
	ADC_InitSturcture.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitSturcture.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitSturcture.ADC_Mode = ADC_Mode_Independent;
	ADC_InitSturcture.ADC_NbrOfChannel = 1;
	ADC_InitSturcture.ADC_ScanConvMode = ENABLE;
	ADC_Init(ADC1, &ADC_InitSturcture);
	ADC_RegularChannelConfig(ADC1,ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);
	ADC_ResetCalibration(ADC1);
	while (ADC_GetResetCalibrationStatus(ADC1) == SET); //等待复位校准完成
	ADC_StartCalibration(ADC1);
	while (ADC_GetCalibrationStatus(ADC1) == SET); //等待校准完成
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	
}
void adc_init(void)
{
	adc_gpio_init();
	adc_lowlevel_init();
	adc_dma_init();
	adc_nvic_init();
	
}
uint16_t adc_read(void)
{
	uint32_t sum;
	adc_dma_init();		//	转化完成一次第二次转化呢要再配一遍dma的参数
	adc_is_busy = 1;
	while (adc_is_busy == 1);
	for (uint8_t i = 0; i < adc_buffer_size; i++)
	{
		sum += adc_buffer[i];
	}
	return sum / adc_buffer_size;
}
void DMA1_Channel1_IRQHandler(void)
{
	if (DMA_GetITStatus(DMA1_IT_TC1) == SET)	//如果DMA转运完成则触发中断
	{
		adc_is_busy = 0;
		DMA_ClearITPendingBit(DMA1_IT_TC1);
	}
}









float AD_to_Temperature(uint16_t ad_value) 
{
  float v_out, rt, temp_k, temp_c;
  
  v_out = (ad_value / AD_RESOLUTION) * V_REF;

  if (v_out < 0.01f || v_out > V_REF - 0.01f) {
    return -99.9f;  // 超出合理范围，返回错误值
  }
  rt = R0 * (v_out / (V_REF - v_out));
  
  temp_k = 1.0f / ( (1.0f / T25) + (1.0f / B_VALUE) * log(rt / R25) );
  temp_c = temp_k - 273.15f;  // 开尔文转摄氏度
  return temp_c;
}


//#define ADC_BUFFER_SIZE 128

//static uint16_t adc_buffer[ADC_BUFFER_SIZE];
//_Bool dma_busy;


//static void adc_io_init(void)
//{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//    GPIO_InitTypeDef GPIO_InitStructure;
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);
//}

//static void adc_lowlevel_init(void)
//{
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//    RCC_ADCCLKConfig(RCC_PCLK2_Div6);
//    ADC_InitTypeDef ADC_InitStructure;
//    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
//    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
//    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
//    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
//    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
//    ADC_InitStructure.ADC_NbrOfChannel = 1;
//    ADC_Init(ADC1, &ADC_InitStructure);

//    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);

//    ADC_DMACmd(ADC1, ENABLE);
//    ADC_Cmd(ADC1, ENABLE);

//    ADC_ResetCalibration(ADC1);
//    while(ADC_GetResetCalibrationStatus(ADC1));
//    ADC_StartCalibration(ADC1);
//    while(ADC_GetCalibrationStatus(ADC1));

//    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
//}

//static void adc_dma_init(void)
//{
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
//    DMA_DeInit(DMA1_Channel1);
//    DMA_InitTypeDef DMA_InitStructure;
//    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
//    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buffer;
//    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
//    DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;
//    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
//    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
//    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
//    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
//    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
//    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
//    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
//    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
//    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
//}

//static void adc_nvic_init(void)
//{
//    NVIC_InitTypeDef NVIC_InitStructure;
//    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);
//}

//void ntc_init(void)
//{
//    adc_io_init();
//    adc_lowlevel_init();
//    adc_dma_init();
//    adc_nvic_init();
//}

//uint16_t ntc_read(void)
//{
//    dma_busy = 1;
//    adc_dma_init();
//    DMA_Cmd(DMA1_Channel1, ENABLE);
//    while (dma_busy);

//    uint32_t sum = 0;
//    for (int i = 0; i < ADC_BUFFER_SIZE; i++)
//    {
//        sum += adc_buffer[i];
//    }
//    return sum / ADC_BUFFER_SIZE;
//}

//void DMA1_Channel1_IRQHandler(void)
//{
//    if (DMA_GetITStatus(DMA1_IT_TC1))
//    {
//        DMA_ClearITPendingBit(DMA1_IT_TC1);
//        dma_busy = 0;
//    }
//}
