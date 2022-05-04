#ifndef __DS18B20_H
#define	__DS18B20_H

#include "stm32f10x.h"

#define LOW 0
#define HIGH 1

#define DS18B20_PORT GPIOB
#define DS18B20_DQ GPIO_Pin_10  //GPIOB_10
#define DS18B20_CLK RCC_APB2Periph_GPIOB

/*****输出高、低电平*****/
#define DS18B20_DATA_OUT(a)	if (a)	\
							GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);	\
				else	\
							GPIO_ResetBits(DS18B20_PORT, DS18B20_DQ)

/*****读取引脚电平*****/
#define DS18B20_DATA_IN()	GPIO_ReadInputDataBit(DS18B20_PORT, DS18B20_DQ)

uint8_t DS18B20_Init(void);
void DS18B20_Write_Byte(uint8_t dat);
uint8_t DS18B20_Read_Byte(void);
float DS18B20_Get_Temp(void);
#endif /* __DS18B20_H */
