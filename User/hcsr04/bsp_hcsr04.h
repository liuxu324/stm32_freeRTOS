#ifndef __HCSR04_H
#define	__HCSR04_H

#include "stm32f10x.h"

#define HCSR04_Trig GPIO_Pin_5  //GPIOB_5
//#define HCSR04_Echo GPIO_Pin_6  //GPIOB_6--TIM4_CH1--通用定时器(需要使用I2C1)
#define HCSR04_Echo GPIO_Pin_1  //GPIOB_1--TIM3_CH4--通用定时器(注：LCD背光口断开)

void HCSR04_Init(void);
uint32_t GetTime_Echo(void);
float HCSR04_GetLength(void);

#endif /* __HCSR04_H */
