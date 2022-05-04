#ifndef __SPI_NRF_H
#define	__SPI_NRF_H

#include "stm32f10x.h"

/************ �ļ������궨�� *************/
#define TX_ADR_WIDTH   5	  // ����/���յ�ַ���
#define TX_PLOAD_WIDTH 5		// ����/��������ͨ����Ч���ݿ�� (5*2) Byte
#define RX_ADR_WIDTH   5
#define RX_PLOAD_WIDTH 5

#define MAX_RT        0x10  // �ط�����жϱ�־λ
#define TX_DS		      0x20  // ���ͳɹ��жϱ�־λ
#define RX_DR		      0x40  // ���ճɹ��жϱ�־λ

/************ NRF ��SPI����궨�� *************/
#define NRF_READ_REG  0x00	// ���Ĵ�������
#define NRF_WRITE_REG 0x20  // д�Ĵ�������
#define DR_RX_PLOAD   0x61  // Define RX payload register address
#define WR_TX_PLOAD   0xA0  // Define TX payload register address
#define FLUSH_TX      0xE1  // Define flush TX register command
#define FLUSH_RX      0xE2  // Define flush RX register command
#define NOP	          0xFF	// �ղ�������״̬�Ĵ���

/************ NRF24L01 ��ؼĴ�����ַ�궨�� *************/
#define CONFIG     0x00 		// 'Config' register address
#define EN_AA      0x01     // 'Enable Auto Acknowledgment' register address
#define EN_RXADDR  0x02     // 'Enabled RX addresses' register address
#define SETUP_RETR 0x04			// 'Setup Auto. Retrans' register address
#define RF_CH      0x05			// 'RF channel' register address
#define RF_SETUP   0x06 		// 'RF setup' register address
#define STATUS     0x07		  // 'Status' register address
#define RX_ADDR_P0 0x0A			// 'RX address pipe0' register address
#define TX_ADDR	   0x10   	// 'TX address' register address// ���ͼĴ����ĵ�ַ
#define RX_PW_P0   0x11 		// 'RX payload width, pipe0' register address

/************************ NRF GPIO�궨�� + �������� *************************/
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

/************************ NRF2 GPIO�궨�� + �������� *************************/
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

/************************ �ļ���������*************************/
void NRF_Test(void);
void NRF_Mode1(void); // Mode1:  NRF - TX_Mode + NRF2 - RX_Mode
void NRF_Mode2(void); // Mode2:  NRF2 - TX_Mode + NRF - RX_Mode

#endif /* __SPI_NRF_H */
