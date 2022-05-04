#ifndef __HY2615_H
#define	__HY2615_H

#include "stm32f10x.h"

/********************** 参数宏定义 *************************/
#define ACK	 		0
#define	NACK 		1
#define I2C_Speed 	(100*1000) 			// IIC传输速率 100kHz

/********************** 地址宏定义 *************************/
#define STM32_ADD 	0x0A 				// Host  address：MCU IIC地址
#define HY2615_ADD 	0x60 				// Slave address：7bit硬件地址

/********************** 命令宏定义 *************************/
#define CMD_ADD0 	0x00 				// 命令字地址 0

/********************* IIC接口 (SMBus总线)***************************/
#define IIC1_PORT GPIOB       
#define IIC1_SCK  GPIO_Pin_6           	// GPIOB_6
#define IIC1_SDA  GPIO_Pin_7           	// GPIOB_7

/********************** GPIO宏定义 *************************/
#define SMBUS_SDA_IN() GPIO_ReadInputDataBit(SMBUS_PORT, SMBUS_SDA) //读取引脚电平

#define	SMBUS_SCK_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SCK_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SDA_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SDA)
#define	SMBUS_SDA_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SDA)

/********************** HY2615 函数声明 *************************/
void HY2615_Init(void);
void HY2615_I2C_Test(void);

#endif /* __HY2615_H */
