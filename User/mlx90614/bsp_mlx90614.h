#ifndef __MLX90614_H
#define	__MLX90614_H

#include "stm32f10x.h"

/********************** 参数宏定义 *************************/
#define ACK	 0
#define	NACK 1

/********************** 地址宏定义 *************************/
#define SA 0x00 				// Slave address：单个MLX90614时为0x00,多个时默认为0x5a

/********************** 命令宏定义 *************************/
#define RAM_ACCESS 0x00 // RAM access command 对 RAM 操作
#define RAM_TOBJ1	 0x07 // To1 address in the eeprom(目标1温度-70.01℃~382.19℃)

/********************* MLX90614引脚 (SMBus总线)***************************/
#define SMBUS_PORT GPIOB       
#define SMBUS_CLK  RCC_APB2Periph_GPIOB  	// 注：使用了 I2C1 的接口
#define SMBUS_SCK  GPIO_Pin_6           	// GPIOB_6
#define SMBUS_SDA  GPIO_Pin_7           	// GPIOB_7

/********************** MLX90614 GPIO宏定义 *************************/
#define SMBUS_SDA_IN() GPIO_ReadInputDataBit(SMBUS_PORT, SMBUS_SDA) //读取引脚电平

#define	SMBUS_SCK_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SCK_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SDA_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SDA)
#define	SMBUS_SDA_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SDA)

/********************** MLX90614 函数声明 *************************/
void SMBus_Init(void);
float SMBus_ReadTemp(void);

#endif /* __MLX90614_H */
