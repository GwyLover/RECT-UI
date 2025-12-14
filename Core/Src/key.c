#include "key.h"
#include "stm32f1xx_hal.h"


uint8_t key_scanf(void){
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3) == 0){
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3) == 0);
		HAL_Delay(20);
		return 1;
	}else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4) == 0){
		HAL_Delay(20);
		while(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4) == 0);
		HAL_Delay(20);
		return 2;
	}
	return 0;
}
