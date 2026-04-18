#ifndef __ADC_H
#define __ADC_H
void adc_init(void);
uint16_t adc_read(void);
float AD_to_Temperature(uint16_t ad_value);




#endif
