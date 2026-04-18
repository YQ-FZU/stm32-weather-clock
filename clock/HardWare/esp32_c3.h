#ifndef __ESP32_C3_H
#define __ESP32_C3_H

void esp_on_usart_received(uint8_t rx_data);
//cmd为需要发送的命令，**rsp存储esp返回的内容，length为返回的数据长度，timeout等待接收完成
static _Bool esp_send_command(const char *cmd, const char **rsp, uint32_t *length, uint32_t timeout);
_Bool esp_send_data(const uint8_t *tx_data, const uint32_t tx_length);
_Bool esp_at_reset(void);
_Bool esp_at_init(void);
_Bool esp_wifi_init(void);
_Bool esp_wifi_connect(const char *ssid, const char *pwd);
_Bool esp_http_get(const char *url, const char **rsp, uint32_t *rx_length, uint32_t timeout);
_Bool esp_sntp_init(void);
_Bool esp_time_get(uint32_t *timestamp);
_Bool esp_wifi_get_ip(char ip[16]);
_Bool esp_wifi_get_mac(char mac[18]);


#endif
