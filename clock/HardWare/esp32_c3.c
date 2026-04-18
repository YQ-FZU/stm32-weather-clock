#include "stm32f10x.h"                  // Device header
#include "esp_usart.h"
#include "./SYSTEM/delay/delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ESP_BUFFER_SIZE		4096
#define RX_RESULT_OK    0
#define RX_RESULT_ERROR 1
#define RX_RESULT_FAIL  2
uint8_t esp_buf[ESP_BUFFER_SIZE];
uint32_t rx_length;		//接收数据长度
_Bool rx_ready;			//1——准备接收，0——未准备
uint8_t rx_result;		//接收结果

void esp_on_usart_received(uint8_t rx_data)
{
	//判断是否准备接收数据
	if (!rx_ready)
	{
		return;
	}
	
	//判断是否溢出
	if (rx_length < ESP_BUFFER_SIZE)
	{
		esp_buf[rx_length++] = rx_data;
		
	}
	else
	{
		rx_result = RX_RESULT_FAIL;		//接收失败
		rx_ready = 0;
		rx_length = 0;					//清空数组
	}
	//判断esp32回复内容
	if (rx_data == '\n')				//数据接收完成
	{
		
		if (rx_length >= 2 && esp_buf[rx_length - 2] == '\r')	//OK\r\n	rx_length = 4
		{
			
			if (rx_length >= 4 && esp_buf[rx_length - 4] == 'O' && esp_buf[rx_length - 3] == 'K')// esp回应OK
			{
				rx_result = RX_RESULT_OK;		
				rx_ready = 0;
			}
			else if (rx_length >= 7 && esp_buf[rx_length - 5] == 'E' && esp_buf[rx_length - 4] == 'R'
					&& esp_buf[rx_length - 3] == 'R' && esp_buf[rx_length - 2] == 'O'
					&& esp_buf[rx_length - 1] == 'R')	//esp返回error
			{
				rx_result = RX_RESULT_ERROR;		
				rx_ready = 0;
			}
		}
	}
}


//cmd为需要发送的命令，**rsp存储esp返回的内容，length为返回的数据长度，timeout等待接收完成
static _Bool esp_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout)
{
	rx_length = 0;					//清空数组
	rx_ready = 1;					//准备接收
	rx_result = RX_RESULT_FAIL;
	esp_usart_send_string(cmd);		//发送命令
	esp_usart_send_string("\r\n");	//发送回车换行
	
	while (timeout--)				//等待串口接收esp数据完成
	{
		delay_ms(1);
	}
	rx_ready = 0;
	if (rsp)
	{
		*rsp = (const char *)esp_buf;
		
	}
	if (length)
	{
		*length = rx_length;
	}
	return rx_result == RX_RESULT_OK;	//判断esp是否返回ok
}


_Bool esp_send_data(const uint8_t *tx_data, const uint32_t tx_length)
{
	esp_usart_send_array(tx_data, tx_length);
	return 1;
}


_Bool esp_at_reset(void)
{
	esp_send_command("AT+RESTORE", NULL, NULL, 1000);
	//这里esp会先返回ready再返回ok
	delay_ms(2000);
	if (!esp_send_command("ATE0", NULL, NULL, 1000))
	{
		Usart_SendString("ATE0 error");
		return 0;
	}
	if (!esp_send_command("AT+SYSSTORE=0", NULL, NULL, 1000))
	{
		Usart_SendString("AT+SYSSTORE=0 error");
		return 0;
	}
	Usart_SendString("esp reset success!");
	return 1;
}


_Bool esp_at_init(void)
{
	rx_ready = 0;
	esp_usart_receive_register(esp_on_usart_received);	//注册回调函数
	esp_usart_init();

	if (esp_at_reset())
	{
		Usart_SendString("esp_at init success!");
		return 1;
	}
	else return 0;
	
}

_Bool esp_wifi_init(void)
{
	if (!esp_send_command("AT+CWMODE=1", NULL, NULL, 1000))	//设置esp为station模式
	{
		Usart_SendString("AT+CWMODE=1 error");	//串口1打印调试信息
		return 0;
	}
	Usart_SendString("wifi init success！");
	return 1;
}

_Bool esp_wifi_connect(const char *ssid, const char *pwd)
{
	char cmd[64];
	snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", ssid, pwd);
	if (!esp_send_command(cmd, NULL, NULL, 5000))	//发送待链接的wifi信息
	{
		Usart_SendString("wifi connect failed");	//串口1打印调试信息
		return 0;
	}
	Usart_SendString("wifi connect success！");
	return 1;
}

_Bool esp_http_get(const char *url, const char **rsp, uint32_t *rx_length, uint32_t timeout)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "AT+HTTPCGET=\"%s\"", url);
	if (!esp_send_command(buf, rsp, rx_length, timeout))
	{
		Usart_SendString("weather get failed");
		return 0;
	}
	Usart_SendString("weather get success");
	return 1;
}
_Bool esp_sntp_init(void)
{
	if (!esp_send_command("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\"", NULL, NULL, 1000))
	{
		Usart_SendString("sntc init failed");
		return 0;
	}
	if (!esp_send_command("AT+CIPSNTPTIME?", NULL, NULL, 1000))
    {
		Usart_SendString("sntc init failed");
        return 0;
    }
		Usart_SendString("sntc init success");
		return 1;
}

_Bool esp_time_get(uint32_t *timestamp)
{
	const char *rsp;
	uint32_t length;
	if (!esp_send_command("AT+SYSTIMESTAMP?", &rsp, &length, 1000))
    {
		Usart_SendString("time get failed");
        return 0;
    }
	char *sts = strstr(rsp, "+SYSTIMESTAMP:") + strlen("+SYSTIMESTAMP:");//找到冒号，并把字符指针定位到冒号之后
	*timestamp = atoi(sts);	//字符串转整数
	Usart_SendString("time get success");
	return 1;
}
_Bool esp_wifi_get_ip(char ip[16])
{
    const char *rsp;
    if (!esp_send_command("AT+CIPSTA?", &rsp, NULL, 1000))
    {
        return 0;
    }

    //解析ip
    const char *pip = strstr(rsp, "+CIPSTA:ip:") + strlen("+CIPSTA:ip:");
    if (pip)
    {
        for (int i = 0; i < 16; i++)
        {
            if (pip[i] == '\r')
            {
                ip[i] = '\0';
                break;
            }
            ip[i] = pip[i];
        }
        return 1;
    }

    return 1;
}

//+CIPSTAMAC:"50:78:7d:47:64:f8"
_Bool esp_wifi_get_mac(char mac[18])
{
    const char *rsp;
    if (!esp_send_command("AT+CIPSTAMAC?", &rsp, NULL, 1000))
    {
        return 0;
    }

    // 解析mac地址
    const char *pmac = strstr(rsp, "+CIPSTAMAC:") + strlen("+CIPSTAMAC:");
    if (pmac)
    {
        strncpy(mac, pmac, 18);
        return 1;
    }

    return 1;
}
