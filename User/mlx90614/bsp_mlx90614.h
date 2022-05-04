#ifndef __MLX90614_H
#define	__MLX90614_H

#include "stm32f10x.h"

/********************** �����궨�� *************************/
#define ACK	 0
#define	NACK 1

/********************** ��ַ�궨�� *************************/
#define SA 0x00 				// Slave address������MLX90614ʱΪ0x00,���ʱĬ��Ϊ0x5a

/********************** ����궨�� *************************/
#define RAM_ACCESS 0x00 // RAM access command �� RAM ����
#define RAM_TOBJ1	 0x07 // To1 address in the eeprom(Ŀ��1�¶�-70.01��~382.19��)

/********************* MLX90614���� (SMBus����)***************************/
#define SMBUS_PORT GPIOB       
#define SMBUS_CLK  RCC_APB2Periph_GPIOB  	// ע��ʹ���� I2C1 �Ľӿ�
#define SMBUS_SCK  GPIO_Pin_6           	// GPIOB_6
#define SMBUS_SDA  GPIO_Pin_7           	// GPIOB_7

/********************** MLX90614 GPIO�궨�� *************************/
#define SMBUS_SDA_IN() GPIO_ReadInputDataBit(SMBUS_PORT, SMBUS_SDA) //��ȡ���ŵ�ƽ

#define	SMBUS_SCK_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SCK_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SCK)
#define	SMBUS_SDA_H()  GPIO_SetBits(SMBUS_PORT, SMBUS_SDA)
#define	SMBUS_SDA_L()  GPIO_ResetBits(SMBUS_PORT, SMBUS_SDA)

/********************** MLX90614 �������� *************************/
void SMBus_Init(void);
float SMBus_ReadTemp(void);

#endif /* __MLX90614_H */
