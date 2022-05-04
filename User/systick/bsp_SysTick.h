#ifndef __SYSTICK_H
#define	__SYSTICK_H

#include "stm32f10x.h"

void SysTick_Init(void);
void TimingDelay_Decrement(void);
void Delay_us(__IO uint32_t nTime);

#define Delay_ms(x) Delay_us(1000*x)	 //��λms

#endif /* __SYSTICK_H */
