#ifndef __HY2615_H
#define	__HY2615_H

#include "stm32f10x.h"

/********************** �����궨�� *************************/
#define ACK	 		0
#define	NACK 		1
#define I2C_Speed 	(100*1000) 			// IIC�������� 100kHz

/********************** ��ַ�궨�� *************************/
#define STM32_ADD 	0x0A 				// Host  address��MCU IIC��ַ
#define HY2615_ADD 	0x60 				// Slave address��7bitӲ����ַ

/********************** ����궨�� *************************/
#define CMD_ADD0 	0x00 				// �����ֵ�ַ 0

/********************* IIC�ӿ� (SMBus����)***************************/
#define IIC1_PORT GPIOB       
#define IIC1_SCK  GPIO_Pin_6           	// GPIOB_6
#define IIC1_SDA  GPIO_Pin_7           	// GPIOB_7

/********************** GPIO�궨�� *************************/
#define SMBUS_SDA_IN() GPIO_ReadInputDataBit(SMBUS_PORT, SMBUS_SDA) //��ȡ���ŵ�ƽ

#define	SMBUS_SCK_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SCK_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SDA_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SDA)
#define	SMBUS_SDA_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SDA)

/********************** HY2615 �������� *************************/
void HY2615_Init(void);
void HY2615_I2C_Test(void);

#endif /* __HY2615_H */
