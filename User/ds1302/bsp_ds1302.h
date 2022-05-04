#ifndef __DS1302_H
#define	__DS1302_H

#include "stm32f10x.h"

/********************** 控制命令宏定义 *************************/
#define WrSec 					0x80 // 
#define WrMin						0x82 // 
#define WrHour 					0x84
#define WrDate 					0x86
#define WrMonth 				0x88
#define WrWeek 					0x8a
#define WrYear 					0x8c
#define WrControl 			0x8e
#define WrTrickleCharge 0x90
#define WrClockBuCE 		0xbe

#define RdSec  					0x81 // 
#define RdMin						0x83 // 
#define RdHour 					0x85
#define RdDate 					0x87
#define RdMonth 				0x89
#define RdWeek 					0x8b
#define RdYear          0x8d
#define RdControl       0x8f
#define RdTrickleCharge 0x91
#define RdClockBuCE     0xbf

/***************** DS1302 引脚 (3根线 -- CE(A0)、DQ(A1)、SCL(A2)) ****************/
#define DS1302_PORT GPIOA       
#define DS1302_CLK  RCC_APB2Periph_GPIOA  // 注：占用了 USART2 的接口
#define DS1302_CE   GPIO_Pin_0           
#define DS1302_DQ   GPIO_Pin_1           
#define DS1302_SCK  GPIO_Pin_2           

/********************** DS1302 GPIO宏定义 *************************/
#define DS1302_DATA_IN() GPIO_ReadInputDataBit(DS1302_PORT, DS1302_DQ) //读取引脚电平

#define	DS1302_CE_H()  GPIO_SetBits(DS1302_PORT, DS1302_CE)
#define	DS1302_CE_L()  GPIO_ResetBits(DS1302_PORT, DS1302_CE)

#define	DS1302_DQ_H()  GPIO_SetBits(DS1302_PORT, DS1302_DQ)
#define	DS1302_DQ_L()  GPIO_ResetBits(DS1302_PORT, DS1302_DQ)

#define	DS1302_SCK_H() GPIO_SetBits(DS1302_PORT, DS1302_SCK)
#define	DS1302_SCK_L() GPIO_ResetBits(DS1302_PORT, DS1302_SCK)

/********************** DS1302 函数声明 *************************/
void DS1302_Init(void);
void DS1302_Time_Show(void);

#endif /* __DS1302_H */
