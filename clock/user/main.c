#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"                  // Device header
#include "LED.h"
#include "key.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/sys/sys.h"
#include "OLED.h"
#include "timer.h"
#include "rtc.h"
#include "adc.h"
#include "esp_usart.h"
#include "esp32_c3.h"
#include "weather.h"
char *wifi_ssid = "Q";
char *wifi_pwd = "13646997019";
char *weather_url = "https://api.seniverse.com/v3/weather/now.json?key=SWTmW0j6KpQUAGv3d&location=beijing&language=en&unit=c";
uint32_t run_s;			//用于计算系统运行时间
uint8_t disp_line_height;	//lcd屏行高
uint8_t KeyNum;			//键码值
uint8_t Usart_Rxbuf[8];//用于缓存串口1接收的数据
uint8_t head, tail; 	//串口缓冲区头尾指针
rtc_date_t my_rtc_date;	//时间信息
uint16_t ad_value;		//温度ad值
float temper;			//温度
uint32_t timestamp;		//时间戳

void timer_elapsed(void)		//定时器回调函数
{
	run_s++;
	if (run_s == 60 * 60 *24)	//超过24小时自动置零
	{
		run_s = 0;
	}
}
void esp_init(void)
{
	lcd_clear(BLACK);
	lcd_show_string(0, disp_line_height, 100, 16, 16, " ESP Init...", WHITE);
	disp_line_height += 16;
	if (!esp_at_init())
	{
		lcd_show_string(0, disp_line_height, 100, 16, 16, " ESP_AT Init Fail", RED);
		disp_line_height += 16;
		while (1);
	}
	lcd_show_string(0, disp_line_height, 100, 16, 16, " Wifi Init...", WHITE);
	disp_line_height += 16;
	if (!esp_wifi_init())
	{
		lcd_show_string(0, disp_line_height, 100, 16, 16, " ESP Wifi Init Fail", RED);
		disp_line_height += 16;
		while (1);
	}
	lcd_show_string(0, disp_line_height, 100, 16, 16, " Wifi Connect...", WHITE);
	disp_line_height += 16;
	if (!esp_wifi_connect(wifi_ssid, wifi_pwd))
	{
		lcd_show_string(0, disp_line_height, 100, 16, 16, " Wifi Connect Fail", RED);
		disp_line_height += 16;
		while (1);
	}
	lcd_show_string(0, disp_line_height, 100, 16, 16, " Sntp Init...", WHITE);
	disp_line_height += 16;
	if (!esp_sntp_init())
	{
		lcd_show_string(0, disp_line_height, 100, 16, 16, " Sntp Init Fail", RED);
		disp_line_height += 16;
		while (1);
	}
	lcd_show_string(0, disp_line_height, 100, 16, 16, " ESP Block Init Success !!!", WHITE);
	disp_line_height += 16;
	lcd_clear(BLACK);
}
	

void Board_Lowlevel_Init(void)
{
	OLED_Init();
	sys_stm32_clock_init(9);    						//设置时钟为72Mhz
    delay_init(72);             						//延时初始化
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//中断优先级分组
	
	lcd_init();
	LED_Init();
	Key_Init();
	Key_Exit_Init();
	adc_init();
	
	rtc_init();
	rtc_set_time(my_rtc_date);
	
	Usart_Init();	//串口1用于调试
	esp_init();
	timer_init();
	delay_ms(2000);
}
//串口1回调函数用于接收串口数据
void On_Usart_Received(uint8_t Rx_Data)
{
	Usart_Rxbuf[head++] = Rx_Data;
	if (head == 8)
	{
		head = 0;
	}
}
//判断缓冲区是否为空
_Bool Usart_rxbuf_is_epity(void)
{
	if (head == tail)
	{
		return 1;
	}
	return 0;
}

int main(void)
{
	char lcd_buf[64];
	_Bool weather_ok = 0;
	_Bool sntp_ok = 0;
	_Bool ip_ok = 0;
	my_rtc_date.year = 2025;
	my_rtc_date.month = 8;
	my_rtc_date.day = 24;
	my_rtc_date.hour = 23;
	my_rtc_date.minute = 3;
	my_rtc_date.second = 0;
	g_point_color = RED;
	g_back_color = BLACK;
	
	//注册回调函数
	Usart_Receive_Register(On_Usart_Received);
	timer_elapsed_register(timer_elapsed);
	
	Board_Lowlevel_Init();//系统初始化
	//const char str[] = "hello world\r\n";
	
	while(1)
	{
		
		//lcd长240，宽320
		//联网同步时间：1小时一次
		if (!sntp_ok  || run_s % 60 == 0)
		{
			sntp_ok = esp_time_get(&timestamp);
			rtc_set_timestamp(timestamp);
			my_rtc_date = rtc_get_time();
		}
		//更新天气信息:10分钟一次
		if (!weather_ok || run_s % (60 * 10) == 0)
		{
			const char *rec;
			weather_ok = esp_http_get(weather_url, &rec, NULL, 10000);
			weather_t weather;
			weather_parse(rec, &weather);
			if (strcmp(weather.weather, "Cloudy") == 0)	//多云
			{
				lcd_show_image(10, 40, 89, 119, 1);
			}
			if (strcmp(weather.weather, "Wind") == 0)	//风
			{
				lcd_show_image(10, 40, 89, 119, 2);
			}
			if (strcmp(weather.weather, "Clear") == 0)	//晴
			{
				lcd_show_image(10, 40, 89, 119, 3);
			}
			if (strcmp(weather.weather, "Snow") == 0)	//雪
			{
				lcd_show_image(10, 40, 89, 119, 4);
			}
			if (strcmp(weather.weather, "Overcase") == 0)	//阴
			{
				lcd_show_image(10, 40, 89, 119, 5);
			}
			if (strcmp(weather.weather, "Rain") == 0)	//雨
			{
				lcd_show_image(10, 40, 89, 119, 6);
			}
			lcd_show_image(10, 40, 89, 119, 1);
			snprintf(lcd_buf, sizeof(lcd_buf), "%s", weather.temperature);
			lcd_show_string( 140, 50, 48, 48,48,lcd_buf,BLUE);
			lcd_show_chinese(190, 50, 0, 44, BLUE);	//℃
		}
		//更新信息
		if (run_s % 1 == 0)
		{
			my_rtc_date = rtc_get_time();
			snprintf(lcd_buf, sizeof(lcd_buf), "%04d-%02d-%02d", my_rtc_date.year, my_rtc_date.month, my_rtc_date.day);
			lcd_show_string(0, 0, 240, 32, 32, lcd_buf, WHITE);		//显示年月日
			lcd_show_chinese(0, 130, 1, 44, YELLOW);	//北
			lcd_show_chinese(45, 130, 2, 44, YELLOW);	//京
			lcd_show_string(91, 130, 100, 48, 48, ":", YELLOW);
			snprintf(lcd_buf, sizeof(lcd_buf), "%02d%s%02d", my_rtc_date.hour, my_rtc_date.second % 2 ? ":" : " ", my_rtc_date.minute);
			lcd_show_string(116, 130, 100, 48, 48, lcd_buf, WHITE);	//显示时分
		}
		//更新环境温度10秒一次
		if (run_s % 10 == 0)
		{
			ad_value = adc_read();
			temper = AD_to_Temperature(ad_value);
			snprintf(lcd_buf, sizeof(lcd_buf), "%02d", (int)temper);
			lcd_show_chinese(96, 100, 3, 32, GREEN);	//室
			lcd_show_chinese(129, 100, 4, 32, GREEN);	//温
			lcd_show_string(162, 100, 16, 32, 32, ":", WHITE);
			lcd_show_string(178, 100, 32, 32, 32, lcd_buf, GREEN);
			lcd_show_chinese(210, 100, 2, 32, GREEN);	//℃
		}
		//更新网络信息30秒一次
		if (!ip_ok || run_s % 30 == 0)
		{
			char ip[16];
			ip_ok = esp_wifi_get_ip(ip);
			lcd_show_chinese(0, 185, 6, 32, RED);
			lcd_show_chinese(33, 185, 7, 32, RED);
			lcd_show_chinese(66, 185, 8, 32, RED);
			lcd_show_chinese(99, 185, 9, 32, RED);
			lcd_show_chinese(132, 185, 5, 32, RED);
			snprintf(lcd_buf, sizeof(lcd_buf), "IP:%s",ip);
			lcd_show_string(0, 230, 152, 16, 16, lcd_buf, WHITE);
			char mac[18];
			esp_wifi_get_mac(mac);
			snprintf(lcd_buf, sizeof(lcd_buf), "MAC:%s",mac);
			lcd_show_string(0, 250, 240, 16, 16, lcd_buf, WHITE);
			snprintf(lcd_buf, sizeof(lcd_buf), "WIFI_SSID:%s", wifi_ssid);
			lcd_show_string(0, 270, 250, 16, 16, lcd_buf, WHITE);
			snprintf(lcd_buf, sizeof(lcd_buf), "WIFI_PWD:%s", wifi_pwd);
			lcd_show_string(0, 290, 250, 16, 16, lcd_buf, WHITE);
		}

		KeyNum = KeyGetNum();
		if (KeyNum == 0)
		{
			LED_ON();
		}
		if (KeyNum == 1)
		{
			LED_OFF();
		}
		//Usart_DMA_Write((uint8_t *)str, strlen(str));
		//===========从串口缓冲区中移出数据并发送=========
		if (Usart_rxbuf_is_epity() != 1)
		{
			Usart_SendByte(Usart_Rxbuf[tail++]);
			if (tail == 8)
			{
				tail = 0;
			}
		}
		//====================================================
	}
}
