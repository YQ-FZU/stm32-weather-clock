#ifndef __ESP_USART_H
#define __ESP_USART_H
#include "stm32f10x.h"                  // Device header
typedef void (*usart_receive_callback_t) (uint8_t data);
void Usart_Init(void);
void Usart_SendByte(uint8_t Byte);
void Usart_SendArray(uint8_t *Array, uint16_t length);
void Usart_SendString(const char *string);
void Usart_DMA_Write(uint8_t *Data, uint16_t DataSize);
void Usart_Receive_Register(usart_receive_callback_t callback);

typedef void (*esp_usart_receive_callback_t) (uint8_t data);
void esp_usart_init(void);
void esp_usart_send_byte(uint8_t Byte);
void esp_usart_send_array(const uint8_t *Array, const uint32_t Length);
void esp_usart_send_string(const char *string);
void esp_usart_receive_register(usart_receive_callback_t esp_callback);


#endif
