/********************************************************************************
  * @file    ����NRF24L01
  * @author  liu xu
  * @date    2017-11-17
  *  �޸ģ�  2018-6-4
	******************************************************************************/ 
#include "bsp_spi_nrf.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

/************************  NRF_Mode ���� ************************/
uint8_t status;
uint8_t Tx_Buf[5] = {0, 1, 2, 3, 4};
uint8_t Rx_Buf[5];
uint8_t i = 0;
/************************  NRF �������� ************************/
uint8_t TX_BUF[TX_PLOAD_WIDTH];	//�������ݻ���
uint8_t RX_BUF[RX_PLOAD_WIDTH];	//�������ݻ���
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};//���徲̬���͵�ַ
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};

   /************************  NRF �������� ************************/
/*** @brief  NRF  ��ʼ����GPIO + Mode�� ***/
void SPI_NRF_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC |
		RCC_APB2Periph_GPIOG|RCC_APB2Periph_AFIO,	ENABLE); //ʹ��GPIIO�����ù���ʱ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //ʹ��SPI1ʱ��
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	//PA.5|6|7	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      // �����������   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;    // SPI�ٶ����10MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_15;// PG.8 | 15		  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // ͨ���������   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 //PC.4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;// ��������   
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	NRF_CSN_HIGH(); // ���ߣ�NRF�������״̬

	SPI_Cmd(SPI1, DISABLE); // ������ʹ�ܣ����ܸı�Mode
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//˫��ȫ˫�� 		  
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	// ��ģʽ  
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS�ź����������
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//8��Ƶ��9MHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // ��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

/*** @brief  NRF -- ��/дһ�ֽ�����
	 * @param  д�������
	 * @retval ����������
****/
uint8_t SPI_NRF_RW(uint8_t dat)
{
	while	(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;	                       // ���ͻ����� �ǿյȴ�
	SPI_I2S_SendData(SPI1, dat);
	
	while	(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		;	                       // ���ջ����� �ǿյȴ�
	
	return SPI_I2S_ReceiveData(SPI1);
}

/** @brief   �� NRF �Ĵ��� -- д�뵥������
  * @param   reg: NRF������ + �Ĵ�����ַ
  *					 dat: ��Ҫ��Ĵ���д�������
  * @retval  NRF �� status �Ĵ�����״̬
***/
uint8_t SPI_NRF_WriteReg(uint8_t reg, uint8_t dat)
{
	uint8_t status;
	NRF_CE_LOW();
	NRF_CSN_LOW();
	
	status = SPI_NRF_RW(reg); // �������� + �Ĵ�����
	SPI_NRF_RW(dat);
	
	NRF_CSN_HIGH();
	return (status);
}

/** @brief   �� NRF �Ĵ��� -- ������������
  * @param   reg: NRF������ + �Ĵ�����ַ
  * @retval  NRF �Ĵ����е�����
***/
uint8_t SPI_NRF_ReadReg(uint8_t reg)
{
	uint8_t reg_val;
	NRF_CE_LOW();
	NRF_CSN_LOW();
	
	SPI_NRF_RW(reg);
	reg_val = SPI_NRF_RW(NOP);
	
	NRF_CSN_HIGH();
	return (reg_val);
}

/** @brief   �� NRF �Ĵ��� -- д��һ������
  * @param   reg  : NRF������ + �Ĵ�����ַ
  *					 pBuf : ���� -- �洢�˽�Ҫд��Ĵ���������
  *          bytes: ����Ĵ�С
  * @retval  NRF �� status �Ĵ�����״̬
***/
uint8_t SPI_NRF_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status = 0, byte_cnt;
	NRF_CE_LOW();		      		// �õ�CE���������ģʽ
	NRF_CSN_LOW();						// �õ�CS��ʹ��SPI����
	
	status = SPI_NRF_RW(reg);	// ���Ĵ�����
	
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		SPI_NRF_RW(*pBuf++);		// д���ݵ�������
	
	NRF_CSN_HIGH();           // �ø�CS�����д����
	return (status);
}

/*** @brief  ��NRF�Ĵ��� -- ����һ������ ***/
uint8_t SPI_NRF_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_cnt;
	NRF_CE_LOW();
	NRF_CSN_LOW();
	
	status = SPI_NRF_RW(reg);
	for (byte_cnt=0;byte_cnt<bytes;byte_cnt++)
		pBuf[byte_cnt] = SPI_NRF_RW(NOP); // ��״̬�Ĵ���
	
	NRF_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- ���� ***/
uint8_t NRF_Check(void)
{
	static uint8_t buf[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	uint8_t buf1[5];
	uint8_t i;
						/**	��1�ֽ�  �����ͳ�����	**/
	SPI_NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);//д��ַ���Ĵ���
	
	SPI_NRF_ReadBuf(TX_ADDR, buf1, 5);								//�ӼĴ���������ַ
	for (i=0;i<5;i++)
	{
		if (buf1[i] != 0xC2)
			break;
	}
	if (5 == i)
		return SUCCESS;
	else
			return ERROR;
}

/*** @brief  NRF -- ���� TX_Mode  ****/
void NRF_TX_Mode(void)
{
	NRF_CE_LOW();
																										 // дTX�ڵ��ַ(Ŀ���ַ)
	SPI_NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	SPI_NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
																										 //����TX�ڵ��ַ����Ҫ��ʹ��NRF-Ack					
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	   //ʹ��ͨ��0���Զ�Ӧ��
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //ʹ��ͨ��0�Ľ��յ�ַ
	SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);//����ط�����:10
//SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	   // ��ʧ��(����--���շֿ�����ʱ)
//SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x00);// ��ʧ��	 
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //����RFͨ��Ϊ:40(���� - CHANAL��
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //0x07(f)?����TX�����������������������
	SPI_NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);    
																				 //���ù���ģʽ����������ģʽ�����������ж�			
	NRF_CE_HIGH();
	Delay_us(10); 
}

/*** @brief  NRF -- ���� RX_Mode  ****/
void NRF_RX_Mode(void)
{
	NRF_CE_LOW();

	SPI_NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);//дRX�ڵ��ַ
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	    //ʹ��ͨ��0���Զ�Ӧ��
//SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	    // ��ʧ��(����--���շֿ�����ʱ)
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);  //ʹ��ͨ��0�Ľ��յ�ַ
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_CH, 40);        //����RFͨ��Ϊ:40(���� - CHANAL��
	SPI_NRF_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ��
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);   //����TX�����������������������
	SPI_NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);     
														//���û�������ģʽ�����PPWR_UP,EN_CRC,16BIT_CRC,����ģʽ
	NRF_CE_HIGH();
}

/** @brief   �� NRF �ķ��ͻ����� -- д������
  * @param   txbuf: ���� -- �����˽�Ҫ���͵�����
  * @retval  ���ͽ���� �ɹ���TX_DS ��ʧ�ܣ�MAX_RT �� ERROR
***/
uint8_t NRF_Tx_Dat(uint8_t *txbuf)
{
	uint8_t state;
	NRF_CE_LOW();
	                                 // д���ݵ�TX_BUF�����32Byte
	SPI_NRF_WriteBuf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
	NRF_CE_HIGH();                   // CE�����أ�TX_BUF�ǿ�ʱ����ʼ�������ݰ�
                                      
	while (NRF_Read_IRQ() != 0)      // �ȴ���������ж� PC.4 -- IRQ
		;
	state = SPI_NRF_ReadReg(STATUS); // ��״̬�Ĵ���
	SPI_NRF_WriteReg(NRF_WRITE_REG + STATUS, state); // ���TX_DS��MAX_RT�жϱ�־
	
	                                 // �ж��ж�����  
	if (state & MAX_RT)              // �ﵽ�ط�����
	{
		SPI_NRF_WriteReg(FLUSH_TX, NOP); // ��� TX �Ĵ���
		return MAX_RT;
	}	
	else if (state & TX_DS)          // �������
		return TX_DS;
	else 
		return ERROR;                  // ����ԭ����ʧ��
}

/** @brief   �� NRF �Ľ��ջ����� -- ��������
  * @param   rxbuf: ���� -- ���ڴ��潫Ҫ���յ�����
  * @retval  ���ս���� �ɹ���RX_DR ��ʧ�ܣ�ERROR
***/
uint8_t NRF_Rx_Dat(uint8_t *rxbuf)
{
	uint8_t state;
	NRF_CE_HIGH();                      // �������״̬����ʼ�������ݰ�
                            
	while (NRF_Read_IRQ() != 0)         // �ȴ������ж� PC.4 -- IRQ
		;
	NRF_CE_LOW();                       // �������״̬
	state = SPI_NRF_ReadReg(STATUS);    // ��״̬�Ĵ���
	SPI_NRF_WriteReg(NRF_WRITE_REG + STATUS, state); // ����жϱ�־
	                 
	if (state & RX_DR)                  // �ж��Ƿ���յ�����    
	{
		SPI_NRF_ReadBuf(DR_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // ��ȡ����
    SPI_NRF_WriteReg(FLUSH_RX, NOP);  // ���RX FIFO�Ĵ���		
		
		return RX_DR;                     // �ɹ����յ�����
	}
	else 
		return ERROR;                     // ��������ʧ��
}

/***************************  NRF2  ************************/
/*** @brief  NRF2 ��ʼ�� ***/
void SPI_NRF2_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
                                                       // ʹ��GPIIO�����ù���ʱ��	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); // ʹ��SPI1ʱ��(����)
																											 // PB.13|14|15	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 // �����������   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_12;// PB.8|12		  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // ͨ���������   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		         // PB.9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        // ��������   
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	NRF2_CSN_HIGH();                                     // ���ߣ�NRF2�������״̬
	
	SPI_Cmd(SPI2, DISABLE);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//˫��ȫ˫�� 		  
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	       // ��ģʽ  
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;   // ��λ��ǰ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE);
}

/*** @brief  NRF2 -- ��/дһ�ֽ�����
	 * @param  д�������
	 * @retval ����������
****/
uint8_t SPI_NRF2_RW(uint8_t dat)
{
	while	(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;	       // ���ͻ����� �ǿյȴ�
	SPI_I2S_SendData(SPI2, dat);
	
	while	(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
		;	       // ���ջ����� �ǿյȴ�
	
	return SPI_I2S_ReceiveData(SPI2);
}

/** @brief   �� NRF2 �Ĵ��� -- д�뵥������
  * @param   reg: NRF������ + �Ĵ�����ַ
  *					 dat: ��Ҫ��Ĵ���д�������
  * @retval  NRF2 �� status �Ĵ�����״̬
***/
uint8_t SPI_NRF2_WriteReg(uint8_t reg, uint8_t dat)
{
	uint8_t status;
	NRF2_CE_LOW();
	NRF2_CSN_LOW();
	
	status = SPI_NRF2_RW(reg);
	SPI_NRF2_RW(dat);
	
	NRF2_CSN_HIGH();
	return (status);
}

/** @brief   �� NRF2 �Ĵ��� -- ������������
  * @param   reg: NRF������ + �Ĵ�����ַ
  * @retval  NRF �Ĵ����е�����
***/
uint8_t SPI_NRF2_ReadReg(uint8_t reg)
{
	uint8_t reg_val;
	NRF2_CE_LOW();
	NRF2_CSN_LOW();
	
	SPI_NRF2_RW(reg);
	reg_val = SPI_NRF2_RW(NOP);
	
	NRF2_CSN_HIGH();
	return (reg_val);
}

/** @brief   �� NRF2 �Ĵ��� -- д��һ������
  * @param   reg  : NRF������ + �Ĵ�����ַ
  *					 pBuf : ���� -- �洢�˽�Ҫд��Ĵ���������
  *          bytes: ����Ĵ�С
  * @retval  NRF �� status �Ĵ�����״̬
***/
uint8_t SPI_NRF2_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status = 0, byte_cnt;
	NRF2_CE_LOW();		   			 // �õ�CE���������ģʽ
	NRF2_CSN_LOW();	  				 // �õ�CS��ʹ��SPI����
	
	status = SPI_NRF2_RW(reg); // ���Ĵ�����
	
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		SPI_NRF2_RW(*pBuf++);		 //	д���ݵ�������
	
	NRF2_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- ��һ������ ***/
uint8_t SPI_NRF2_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_cnt;
	NRF2_CE_LOW();
	NRF2_CSN_LOW();
	
	status = SPI_NRF2_RW(reg);
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		pBuf[byte_cnt] = SPI_NRF2_RW(NOP); // ��״̬�Ĵ���
	
	NRF2_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- ���� ***/
uint8_t NRF2_Check(void)
{
	static uint8_t buf[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	uint8_t buf1[5];
	uint8_t i;
				/**	��1�ֽ�  �����ͳ�����	**/
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);//д��ַ���Ĵ���
	
	SPI_NRF2_ReadBuf(TX_ADDR, buf1, 5);								//�ӼĴ���������ַ
	for (i=0; i<5; i++)
	{
		if (buf1[i] != 0xC2)
			break;
	}
	if (5 == i)
		return SUCCESS;
	else
			return ERROR;
}

/*** @brief  NRF2 -- �ӻ� RX_Mode  ****/
void NRF2_TX_Mode(void)
{
	NRF2_CE_LOW();
																										 // дTX�ڵ��ַ(Ŀ���ַ)
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
																										 //����TX�ڵ��ַ����Ҫ��ʹ��NRF-Ack					
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	   //ʹ��ͨ��0���Զ�Ӧ��
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //ʹ��ͨ��0�Ľ��յ�ַ
	SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);//����ط�����:10
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	   // ��ʧ��(����--���շֿ�����ʱ)
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); // ��ʧ��
//SPI_NRF2_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x00);// ��ʧ��	
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //����RFͨ��Ϊ:40(���� - CHANAL��
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //����TX�����������������������
	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);    
																				 //���ù���ģʽ����������ģʽ�����������ж�			
	NRF2_CE_HIGH();
	Delay_us(10); 
}

/*** @brief  NRF2 -- �ӻ� RX_Mode  ****/
void NRF2_RX_Mode(void)
{
	NRF2_CE_LOW();

	SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);//дRX�ڵ��ַ
	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	    //ʹ��ͨ��0���Զ�Ӧ��
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	    // ��ʧ��(����--���շֿ�����ʱ)
	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //ʹ��ͨ��0�Ľ��յ�ַ
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //����RFͨ��Ϊ:40(���� - CHANAL��
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);//ѡ��ͨ��0����Ч���ݿ��
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //0x07/f����TX�����������������������
	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);     
														//���û�������ģʽ�����PPWR_UP,EN_CRC,16BIT_CRC,����ģʽ
	NRF2_CE_HIGH();
}

/** @brief   �� NRF2 �ķ��ͻ����� -- д������
  * @param   txbuf: ���� -- �����˽�Ҫ���͵�����
  * @retval  ���ͽ���� �ɹ���TX_DS ��ʧ�ܣ�MAX_RT �� ERROR
***/
uint8_t NRF2_Tx_Dat(uint8_t *txbuf)
{
	uint8_t state;
	NRF2_CE_LOW();
	                                   // д���ݵ�TX_BUF�����32Byte
	SPI_NRF2_WriteBuf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
	NRF2_CE_HIGH();                    // CE�����أ�TX_BUF�ǿ�ʱ����ʼ�������ݰ�
                                      
	while (NRF2_Read_IRQ() != 0)       // �ȴ���������ж� PB.9 -- IRQ
		;
	state = SPI_NRF2_ReadReg(STATUS);  // ��״̬�Ĵ���
	SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state); // ���TX_DS��MAX_RT�жϱ�־

	                                   // �ж��ж�����  
	if (state & MAX_RT)                // �ﵽ�ط�����
	{
		SPI_NRF2_WriteReg(FLUSH_TX, NOP);// ��� TX �Ĵ���
		return MAX_RT;
	}	
	else if (state & TX_DS)            // �������
		return TX_DS;
	else 
		return ERROR;                    // ����ԭ����ʧ��
}

/** @brief   �� NRF2 �Ľ��ջ����� -- ��������
  * @param   rxbuf: ���� -- ���ڴ��潫Ҫ���յ�����
  * @retval  ���ս���� �ɹ���RX_DR ��ʧ�ܣ�ERROR
***/
uint8_t NRF2_Rx_Dat(uint8_t *rxbuf)
{
	uint8_t state;
	NRF2_CE_HIGH();                     // �������״̬����ʼ�������ݰ�
                            
	while (NRF2_Read_IRQ() != 0)        // �ȴ������ж� PB.9 -- IRQ
		;
	NRF2_CE_LOW();                      // �������״̬
	state = SPI_NRF2_ReadReg(STATUS);   // ��״̬�Ĵ���
	SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state); // ����жϱ�־
	                 
	if (state & RX_DR)                  // �ж��Ƿ���յ�����    
	{
		SPI_NRF2_ReadBuf(DR_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // ��ȡ����
    SPI_NRF2_WriteReg(FLUSH_RX, NOP); // ���RX FIFO�Ĵ���		
		
		return RX_DR;                     // �ɹ����յ�����
	}
	else 
		return ERROR;                     // ��������ʧ��
}

/*** @brief  NRF -- ��⺯�� ***/
void NRF_Test(void)
{
	static uint8_t status = 0;
	
	status = NRF_Check();
	if (status)			printf("\nNRF-Success!");
	else			      printf("\nNRF-Wait...!");

	status = NRF2_Check();
	if (status)			printf("\tNRF2-Success!");
	else       			printf("\tNRF2-Wait...!");
}

/***************************** NRF_Mode *****************************/
/*** @brief Mode1:  NRF - TX_Mode + NRF2 - RX_Mode ***/
void NRF_Mode1(void)
{
	NRF_TX_Mode();					      // ��������ģʽ NRF  - TX_Mode
	NRF2_RX_Mode();					      // �ӻ�����ģʽ NRF2 - RX_Mode
		
	status = NRF_Tx_Dat(Tx_Buf);  // ��ʼ��������
	switch (status)               // �жϷ���״̬
	{
		case MAX_RT: printf("\nNRF1: has no Ack, number > MAX_RT --> Faule!");
			break;                    // ��Ӧ��ʧ��
		case ERROR:	printf("\nNRF1:Donot konw why --> Faul!");
			break;                    // δ֪ԭ��ʧ��
		case TX_DS:	printf("\nNRF1:TX and Rx Success!\n");
			break;                    // ���ͳɹ�
 	}
		
	status = NRF2_Rx_Dat(Rx_Buf); // �ȴ���������
  if (status == RX_DR)
	{
		printf("NRF --> NRF2 Data:");
		for (i=0; i<5; i++)
		{
			printf("\t%d", Rx_Buf[i]);
			Rx_Buf[i] += 1;
			Tx_Buf[i] = Rx_Buf[i];
		}
	}
	else 
		printf("\nERROR!");
}

/*** @brief Mode2:  NRF2 - TX_Mode + NRF - RX_Mode ***/
void NRF_Mode2(void)
{
	NRF2_TX_Mode();					     // NRF2 - TX_Mode
	NRF_RX_Mode();					     // NRF  - RX_Mode

  status = NRF2_Tx_Dat(Tx_Buf);// ��ʼ��������
	switch (status)              // �жϷ���״̬
	{
		case MAX_RT: printf("\nNRF2: has no Ack, number > MAX_RT --> Faule!");
			break;                   // ��Ӧ��ʧ��
		case ERROR:	printf("\nNRF2:Donot konw why --> Faul!");
			break;                   // δ֪ԭ��ʧ��
		case TX_DS:	printf("\nNRF2:TX and Rx Success!\n");
			break;                   // ���ͳɹ�
 	}
		
	status = NRF_Rx_Dat(Rx_Buf); // �ȴ���������
  if (status == RX_DR)
	{
		printf("NRF2 --> NRF Data:");
		for (i=0; i<5; i++)
		{
			printf("\t%d", Rx_Buf[i]);
			Rx_Buf[i] += 1;
			Tx_Buf[i] = Rx_Buf[i];
		} puts("\n");
	}
	else 
		printf("\nERROR!");
}

/*********************************************END OF FILE**********************/
