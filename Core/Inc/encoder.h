#ifndef __ENCODER_H__
#define __ENCODER_H__
#include "stm32f1xx_hal.h"

void encoder_init(void);
int16_t get_cnt(void);

#endif
