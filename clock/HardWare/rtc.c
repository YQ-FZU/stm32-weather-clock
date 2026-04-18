#include "stm32f10x.h"                  // Device header
#include "rtc.h"
#include "time.h"
void rtc_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	 if (BKP_ReadBackupRegister(BKP_DR1) != 0xA5A5)
	 {
		RCC_LSEConfig(RCC_LSE_ON);
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) != SET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForSynchro();		//等待时钟同步
		RTC_WaitForLastTask();
		RTC_SetPrescaler(32768-1);	//对于lse为32.76khz的时钟进行分频，使得一秒cnt位数加一
		RTC_WaitForLastTask();
		BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
	 }
	else 
	{
		RTC_WaitForSynchro();								
		RTC_WaitForLastTask();
	}
}
void rtc_set_time(rtc_date_t my_time)
{
	time_t time_cnt;		//定义秒计数器数据类型
	struct tm time_date;	//定义日期时间数据类型
    time_date.tm_year = my_time.year - 1900;		//将数组的时间赋值给日期时间结构体
	time_date.tm_mon = my_time.month - 1;
	time_date.tm_mday = my_time.day;
	time_date.tm_hour = my_time.hour;
	time_date.tm_min = my_time.minute;
	time_date.tm_sec = my_time.second;
	time_cnt = mktime(&time_date) - 8 * 60 * 60;	//调用mktime函数，将日期时间转换为秒计数器格式
													//- 8 * 60 * 60为东八区的时区调整
    RTC_SetCounter(time_cnt);//写入cnt计数器
    RTC_WaitForLastTask();							//等待上一次操作完成
}
void rtc_set_timestamp(uint32_t timestamp)
{
	RTC_SetCounter(timestamp);//写入cnt计数器
    RTC_WaitForLastTask();
}
rtc_date_t rtc_get_time(void)
{
	rtc_date_t my_date;
	time_t time_cnt;
	time_cnt = RTC_GetCounter() + 8 * 60 * 60;
	struct tm time_date;
	time_date = *localtime(&time_cnt);
	my_date.year =  time_date.tm_year + 1900;
	my_date.month =  time_date.tm_mon + 1;
	my_date.day =  time_date.tm_mday;
	my_date.hour =  time_date.tm_hour;
	my_date.minute =  time_date.tm_min;
	my_date.second =  time_date.tm_sec;
	return my_date;
}
