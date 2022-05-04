#ifndef __DHT11_H
#define	__DHT11_H

#include "stm32f10x.h"

/*** @brief  定义存放温湿度数据的结构体 DHT11_Data_Typedef ***/
typedef struct
{
	uint8_t humi_int;  // 湿度的整数部分
	uint8_t humi_deci; // 湿度的小数部分 
	uint8_t temp_int;  // 温度的整数部分
	uint8_t temp_deci; // 温度的小数部分
	uint8_t check_sum; // 校验和
}DHT11_Data_TypeDef;

#define LOW  0
#define HIGH 1

#define DHT11_PORT GPIOB
#define DHT11_DQ   GPIO_Pin_10  //GPIOB_10
#define DHT11_CLK  RCC_APB2Periph_GPIOB

/*****输出高、低电平*****/
#define DHT11_DATA_OUT(a)	if (a)	\
							GPIO_SetBits(DHT11_PORT, DHT11_DQ);	\
				else	\
							GPIO_ResetBits(DHT11_PORT, DHT11_DQ)

/*****读取引脚电平*****/
#define DHT11_DATA_IN()	GPIO_ReadInputDataBit(DHT11_PORT, DHT11_DQ)

void DHT11_GPIO_Config(void);
void Show_DHT11(void);

#endif /* __DHT11_H */
