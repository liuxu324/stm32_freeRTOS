#ifndef __SPI_NRF_H
#define	__SPI_NRF_H

#include "stm32f10x.h"

/************ 文件参数宏定义 *************/
#define TX_ADR_WIDTH   5	  // 发射/接收地址宽度
#define TX_PLOAD_WIDTH 5		// 发射/接收数据通道有效数据宽度 (5*2) Byte
#define RX_ADR_WIDTH   5
#define RX_PLOAD_WIDTH 5

#define MAX_RT        0x10  // 重发溢出中断标志位
#define TX_DS		      0x20  // 发送成功中断标志位
#define RX_DR		      0x40  // 接收成功中断标志位

/************ NRF 的SPI命令宏定义 *************/
#define NRF_READ_REG  0x00	// 读寄存器命令
#define NRF_WRITE_REG 0x20  // 写寄存器命令
#define DR_RX_PLOAD   0x61  // Define RX payload register address
#define WR_TX_PLOAD   0xA0  // Define TX payload register address
#define FLUSH_TX      0xE1  // Define flush TX register command
#define FLUSH_RX      0xE2  // Define flush RX register command
#define NOP	          0xFF	// 空操作，读状态寄存器

/************ NRF24L01 相关寄存器地址宏定义 *************/
#define CONFIG     0x00 		// 'Config' register address
#define EN_AA      0x01     // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR  0x02     // 'Enabled RX addresses' register address
#define SETUP_RETR 0x04			// 'Setup Auto. Retrans' register address
#define RF_CH      0x05			// 'RF channel' register address
#define RF_SETUP   0x06 		// 'RF setup' register address
#define STATUS     0x07		  // 'Status' register address
#define RX_ADDR_P0 0x0A			// 'RX address pipe0' register address
#define TX_ADDR	   0x10   	// 'TX address' register address// 发送寄存器的地址
#define RX_PW_P0   0x11 		// 'RX payload width, pipe0' register address

/************************ NRF GPIO宏定义 + 函数声明 *************************/
#define NRF_Read_IRQ() GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_4)	//IRQ -- PC.4
#define	NRF_CE_HIGH()  GPIO_SetBits(GPIOG, GPIO_Pin_8)
#define	NRF_CE_LOW()   GPIO_ResetBits(GPIOG, GPIO_Pin_8)
#define	NRF_CSN_HIGH() GPIO_SetBits(GPIOG, GPIO_Pin_15)
#define	NRF_CSN_LOW()  GPIO_ResetBits(GPIOG, GPIO_Pin_15)

void SPI_NRF_Init(void);
void NRF_TX_Mode(void);
void NRF_RX_Mode(void);
uint8_t NRF_Tx_Dat(uint8_t *txbuf);
uint8_t NRF_Rx_Dat(uint8_t *rxbuf);

/************************ NRF2 GPIO宏定义 + 函数声明 *************************/
#define NRF2_Read_IRQ() GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9)	//IRQ -- PB.9
#define	NRF2_CE_HIGH()  GPIO_SetBits(GPIOB, GPIO_Pin_8)
#define	NRF2_CE_LOW()   GPIO_ResetBits(GPIOB, GPIO_Pin_8)
#define	NRF2_CSN_HIGH() GPIO_SetBits(GPIOB, GPIO_Pin_12)
#define	NRF2_CSN_LOW()  GPIO_ResetBits(GPIOB, GPIO_Pin_12)

void SPI_NRF2_Init(void);
void NRF2_TX_Mode(void);
void NRF2_RX_Mode(void);
uint8_t NRF2_Tx_Dat(uint8_t *txbuf);
uint8_t NRF2_Rx_Dat(uint8_t *rxbuf);

/************************ 文件函数声明*************************/
void NRF_Test(void);
void NRF_Mode1(void); // Mode1:  NRF - TX_Mode + NRF2 - RX_Mode
void NRF_Mode2(void); // Mode2:  NRF2 - TX_Mode + NRF - RX_Mode

#endif /* __SPI_NRF_H */
