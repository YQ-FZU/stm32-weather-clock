#ifndef __RTC_H
#define __RTC_H
typedef struct
{
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
}rtc_date_t;
void rtc_init(void);
void rtc_set_time(rtc_date_t my_time);
rtc_date_t rtc_get_time(void);
void rtc_set_timestamp(uint32_t timestamp);


#endif
