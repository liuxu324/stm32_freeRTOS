#ifndef __KEY_H
#define	__KEY_H

#include "stm32f10x.h"

/** the GPIO definition of key 
  * KEY2 - PA0
  * KEY3 - PC13
  */
  
#define KEY_ON	(0)
#define KEY_OFF	(1)

#define KEY2_GPIO_PORT	(GPIOA)
#define KEY2_GPIO_PIN	(GPIO_Pin_0)

#define KEY3_GPIO_PORT	(GPIOC)
#define KEY3_GPIO_PIN	(GPIO_Pin_13)

uint8_t Key_Scan(GPIO_TypeDef* GPIOx, u16 GPIO_Pin);
void KEY_GPIO_Config(void);

#endif /* __KEY_H */

