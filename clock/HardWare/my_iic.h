#ifndef __MY_IIC_H
#define __MY_IIC_H
void my_iic_init(void);
void my_iic_start(void);
void my_iic_stop(void);
void my_iic_send_byte(uint8_t byte);
uint8_t my_iic_receive_byte(void);
void my_iic_send_ack(uint8_t ack);
uint8_t my_iic_receive_ack(void);

#endif
