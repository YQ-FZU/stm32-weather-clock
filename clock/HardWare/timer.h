#ifndef __TIMER_H
#define __TIMER_H
#include <stdbool.h>
#include <stdint.h>
extern uint8_t flag;
typedef void (*timer_elapsed_callback_t)(void);
void timer_init(void);
void timer_elapsed_register(timer_elapsed_callback_t callback);


#endif
