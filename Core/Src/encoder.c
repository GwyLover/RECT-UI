#include "stm32f1xx_hal.h"
#include "encoder.h"
#include "tim.h"

void encoder_init(void){
	HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_ALL);
}

int16_t get_cnt(void){
	int16_t num = __HAL_TIM_GetCounter(&htim3);
	__HAL_TIM_SetCounter(&htim3,0);
	return num;
}
