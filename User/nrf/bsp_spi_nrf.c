/********************************************************************************
  * @file    调试NRF24L01
  * @author  liu xu
  * @date    2017-11-17
  *  修改：  2018-6-4
	******************************************************************************/ 
#include "bsp_spi_nrf.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

/************************  NRF_Mode 参数 ************************/
uint8_t status;
uint8_t Tx_Buf[5] = {0, 1, 2, 3, 4};
uint8_t Rx_Buf[5];
uint8_t i = 0;
/************************  NRF 参数部分 ************************/
uint8_t TX_BUF[TX_PLOAD_WIDTH];	//发送数据缓存
uint8_t RX_BUF[RX_PLOAD_WIDTH];	//接收数据缓存
uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};//定义静态发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0x01};

   /************************  NRF 函数部分 ************************/
/*** @brief  NRF  初始化（GPIO + Mode） ***/
void SPI_NRF_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC |
		RCC_APB2Periph_GPIOG|RCC_APB2Periph_AFIO,	ENABLE); //使能GPIIO、复用功能时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); //使能SPI1时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	//PA.5|6|7	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;      // 复用推挽输出   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;    // SPI速度最大10MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_15;// PG.8 | 15		  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // 通用推挽输出   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 //PC.4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;// 上拉输入   
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	NRF_CSN_HIGH(); // 拉高，NRF进入空闲状态

	SPI_Cmd(SPI1, DISABLE); // 必须先使能，才能改变Mode
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//双线全双工 		  
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	// 主模式  
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft; //NSS信号由软件产生
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;//8分频，9MHz
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB; // 高位在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI1, &SPI_InitStructure);
	
	SPI_Cmd(SPI1, ENABLE);
}

/*** @brief  NRF -- 读/写一字节数据
	 * @param  写入的数据
	 * @retval 读到的数据
****/
uint8_t SPI_NRF_RW(uint8_t dat)
{
	while	(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
		;	                       // 发送缓冲器 非空等待
	SPI_I2S_SendData(SPI1, dat);
	
	while	(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
		;	                       // 接收缓冲器 非空等待
	
	return SPI_I2S_ReceiveData(SPI1);
}

/** @brief   向 NRF 寄存器 -- 写入单个数据
  * @param   reg: NRF的命令 + 寄存器地址
  *					 dat: 将要向寄存器写入的数据
  * @retval  NRF 的 status 寄存器的状态
***/
uint8_t SPI_NRF_WriteReg(uint8_t reg, uint8_t dat)
{
	uint8_t status;
	NRF_CE_LOW();
	NRF_CSN_LOW();
	
	status = SPI_NRF_RW(reg); // 发送命令 + 寄存器号
	SPI_NRF_RW(dat);
	
	NRF_CSN_HIGH();
	return (status);
}

/** @brief   从 NRF 寄存器 -- 读出单个数据
  * @param   reg: NRF的命令 + 寄存器地址
  * @retval  NRF 寄存器中的数据
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

/** @brief   向 NRF 寄存器 -- 写入一串数据
  * @param   reg  : NRF的命令 + 寄存器地址
  *					 pBuf : 数组 -- 存储了将要写入寄存器的数据
  *          bytes: 数组的大小
  * @retval  NRF 的 status 寄存器的状态
***/
uint8_t SPI_NRF_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status = 0, byte_cnt;
	NRF_CE_LOW();		      		// 置低CE，进入待机模式
	NRF_CSN_LOW();						// 置低CS，使能SPI传输
	
	status = SPI_NRF_RW(reg);	// 发寄存器号
	
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		SPI_NRF_RW(*pBuf++);		// 写数据到缓冲区
	
	NRF_CSN_HIGH();           // 置高CS，完成写数据
	return (status);
}

/*** @brief  从NRF寄存器 -- 读出一串数据 ***/
uint8_t SPI_NRF_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_cnt;
	NRF_CE_LOW();
	NRF_CSN_LOW();
	
	status = SPI_NRF_RW(reg);
	for (byte_cnt=0;byte_cnt<bytes;byte_cnt++)
		pBuf[byte_cnt] = SPI_NRF_RW(NOP); // 读状态寄存器
	
	NRF_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- 测试 ***/
uint8_t NRF_Check(void)
{
	static uint8_t buf[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	uint8_t buf1[5];
	uint8_t i;
						/**	第1字节  被解释成命令	**/
	SPI_NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);//写地址到寄存器
	
	SPI_NRF_ReadBuf(TX_ADDR, buf1, 5);								//从寄存器读出地址
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

/*** @brief  NRF -- 主机 TX_Mode  ****/
void NRF_TX_Mode(void)
{
	NRF_CE_LOW();
																										 // 写TX节点地址(目标地址)
	SPI_NRF_WriteBuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	SPI_NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
																										 //设置TX节点地址，主要是使能NRF-Ack					
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	   //使能通道0的自动应答
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址
	SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);//最大重发次数:10
//SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	   // 先失能(发送--接收分开调试时)
//SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x00);// 先失能	 
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //设置RF通道为:40(参数 - CHANAL）
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //0x07(f)?设置TX发射参数，开启低噪声增益
	SPI_NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);    
																				 //配置工作模式参数，发射模式，开启所有中断			
	NRF_CE_HIGH();
	Delay_us(10); 
}

/*** @brief  NRF -- 主机 RX_Mode  ****/
void NRF_RX_Mode(void)
{
	NRF_CE_LOW();

	SPI_NRF_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);//写RX节点地址
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	    //使能通道0的自动应答
//SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	    // 先失能(发送--接收分开调试时)
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01);  //使能通道0的接收地址
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_CH, 40);        //设置RF通道为:40(参数 - CHANAL）
	SPI_NRF_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);//选择通道0的有效数据宽度
	SPI_NRF_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);   //设置TX发射参数，开启低噪声增益
	SPI_NRF_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);     
														//配置基本工作模式参数PWR_UP,EN_CRC,16BIT_CRC,接收模式
	NRF_CE_HIGH();
}

/** @brief   向 NRF 的发送缓冲区 -- 写入数据
  * @param   txbuf: 数组 -- 储存了将要发送的数据
  * @retval  发送结果， 成功：TX_DS ；失败：MAX_RT 或 ERROR
***/
uint8_t NRF_Tx_Dat(uint8_t *txbuf)
{
	uint8_t state;
	NRF_CE_LOW();
	                                 // 写数据到TX_BUF，最大32Byte
	SPI_NRF_WriteBuf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
	NRF_CE_HIGH();                   // CE上跳沿，TX_BUF非空时，开始发送数据包
                                      
	while (NRF_Read_IRQ() != 0)      // 等待发送完成中断 PC.4 -- IRQ
		;
	state = SPI_NRF_ReadReg(STATUS); // 读状态寄存器
	SPI_NRF_WriteReg(NRF_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志
	
	                                 // 判断中断类型  
	if (state & MAX_RT)              // 达到重发次数
	{
		SPI_NRF_WriteReg(FLUSH_TX, NOP); // 清除 TX 寄存器
		return MAX_RT;
	}	
	else if (state & TX_DS)          // 发送完成
		return TX_DS;
	else 
		return ERROR;                  // 其他原因发送失败
}

/** @brief   从 NRF 的接收缓冲区 -- 读出数据
  * @param   rxbuf: 数组 -- 用于储存将要接收的数据
  * @retval  接收结果， 成功：RX_DR ；失败：ERROR
***/
uint8_t NRF_Rx_Dat(uint8_t *rxbuf)
{
	uint8_t state;
	NRF_CE_HIGH();                      // 进入接收状态，开始接收数据包
                            
	while (NRF_Read_IRQ() != 0)         // 等待接收中断 PC.4 -- IRQ
		;
	NRF_CE_LOW();                       // 进入待机状态
	state = SPI_NRF_ReadReg(STATUS);    // 读状态寄存器
	SPI_NRF_WriteReg(NRF_WRITE_REG + STATUS, state); // 清除中断标志
	                 
	if (state & RX_DR)                  // 判断是否接收到数据    
	{
		SPI_NRF_ReadBuf(DR_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // 读取数据
    SPI_NRF_WriteReg(FLUSH_RX, NOP);  // 清除RX FIFO寄存器		
		
		return RX_DR;                     // 成功接收到数据
	}
	else 
		return ERROR;                     // 接收数据失败
}

/***************************  NRF2  ************************/
/*** @brief  NRF2 初始化 ***/
void SPI_NRF2_Init(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
                                                       // 使能GPIIO、复用功能时钟	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE); // 使能SPI1时钟(低速)
																											 // PB.13|14|15	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;			 // 复用推挽输出   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_12;// PB.8|12		  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // 通用推挽输出   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		         // PB.9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        // 上拉输入   
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	NRF2_CSN_HIGH();                                     // 拉高，NRF2进入空闲状态
	
	SPI_Cmd(SPI2, DISABLE);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;//双线全双工 		  
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	       // 主模式  
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;   // 高位在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI2, &SPI_InitStructure);
	
	SPI_Cmd(SPI2, ENABLE);
}

/*** @brief  NRF2 -- 读/写一字节数据
	 * @param  写入的数据
	 * @retval 读到的数据
****/
uint8_t SPI_NRF2_RW(uint8_t dat)
{
	while	(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)
		;	       // 发送缓冲器 非空等待
	SPI_I2S_SendData(SPI2, dat);
	
	while	(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)
		;	       // 接收缓冲器 非空等待
	
	return SPI_I2S_ReceiveData(SPI2);
}

/** @brief   向 NRF2 寄存器 -- 写入单个数据
  * @param   reg: NRF的命令 + 寄存器地址
  *					 dat: 将要向寄存器写入的数据
  * @retval  NRF2 的 status 寄存器的状态
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

/** @brief   从 NRF2 寄存器 -- 读出单个数据
  * @param   reg: NRF的命令 + 寄存器地址
  * @retval  NRF 寄存器中的数据
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

/** @brief   向 NRF2 寄存器 -- 写入一串数据
  * @param   reg  : NRF的命令 + 寄存器地址
  *					 pBuf : 数组 -- 存储了将要写入寄存器的数据
  *          bytes: 数组的大小
  * @retval  NRF 的 status 寄存器的状态
***/
uint8_t SPI_NRF2_WriteBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status = 0, byte_cnt;
	NRF2_CE_LOW();		   			 // 置低CE，进入待机模式
	NRF2_CSN_LOW();	  				 // 置低CS，使能SPI传输
	
	status = SPI_NRF2_RW(reg); // 发寄存器号
	
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		SPI_NRF2_RW(*pBuf++);		 //	写数据到缓冲区
	
	NRF2_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- 读一串数据 ***/
uint8_t SPI_NRF2_ReadBuf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	uint8_t status, byte_cnt;
	NRF2_CE_LOW();
	NRF2_CSN_LOW();
	
	status = SPI_NRF2_RW(reg);
	for (byte_cnt=0; byte_cnt<bytes; byte_cnt++)
		pBuf[byte_cnt] = SPI_NRF2_RW(NOP); // 读状态寄存器
	
	NRF2_CSN_HIGH();
	return (status);
}

/*** @brief  NRF -- 测试 ***/
uint8_t NRF2_Check(void)
{
	static uint8_t buf[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
	uint8_t buf1[5];
	uint8_t i;
				/**	第1字节  被解释成命令	**/
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, buf, 5);//写地址到寄存器
	
	SPI_NRF2_ReadBuf(TX_ADDR, buf1, 5);								//从寄存器读出地址
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

/*** @brief  NRF2 -- 从机 RX_Mode  ****/
void NRF2_TX_Mode(void)
{
	NRF2_CE_LOW();
																										 // 写TX节点地址(目标地址)
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + TX_ADDR, TX_ADDRESS, TX_ADR_WIDTH);
	SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);
																										 //设置TX节点地址，主要是使能NRF-Ack					
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	   //使能通道0的自动应答
	SPI_NRF_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址
	SPI_NRF_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x1a);//最大重发次数:10
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	   // 先失能(发送--接收分开调试时)
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); // 先失能
//SPI_NRF2_WriteReg(NRF_WRITE_REG + SETUP_RETR, 0x00);// 先失能	
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //设置RF通道为:40(参数 - CHANAL）
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //设置TX发射参数，开启低噪声增益
	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0e);    
																				 //配置工作模式参数，发射模式，开启所有中断			
	NRF2_CE_HIGH();
	Delay_us(10); 
}

/*** @brief  NRF2 -- 从机 RX_Mode  ****/
void NRF2_RX_Mode(void)
{
	NRF2_CE_LOW();

	SPI_NRF2_WriteBuf(NRF_WRITE_REG + RX_ADDR_P0, RX_ADDRESS, RX_ADR_WIDTH);//写RX节点地址
	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x01);	    //使能通道0的自动应答
//SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_AA, 0x00);	    // 先失能(发送--接收分开调试时)
	SPI_NRF2_WriteReg(NRF_WRITE_REG + EN_RXADDR, 0x01); //使能通道0的接收地址
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_CH, 40);       //设置RF通道为:40(参数 - CHANAL）
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RX_PW_P0, RX_PLOAD_WIDTH);//选择通道0的有效数据宽度
	SPI_NRF2_WriteReg(NRF_WRITE_REG + RF_SETUP, 0x0f);  //0x07/f设置TX发射参数，开启低噪声增益
	SPI_NRF2_WriteReg(NRF_WRITE_REG + CONFIG, 0x0f);     
														//配置基本工作模式参数PWR_UP,EN_CRC,16BIT_CRC,接收模式
	NRF2_CE_HIGH();
}

/** @brief   向 NRF2 的发送缓冲区 -- 写入数据
  * @param   txbuf: 数组 -- 储存了将要发送的数据
  * @retval  发送结果， 成功：TX_DS ；失败：MAX_RT 或 ERROR
***/
uint8_t NRF2_Tx_Dat(uint8_t *txbuf)
{
	uint8_t state;
	NRF2_CE_LOW();
	                                   // 写数据到TX_BUF，最大32Byte
	SPI_NRF2_WriteBuf(WR_TX_PLOAD, txbuf, TX_PLOAD_WIDTH);
	NRF2_CE_HIGH();                    // CE上跳沿，TX_BUF非空时，开始发送数据包
                                      
	while (NRF2_Read_IRQ() != 0)       // 等待发送完成中断 PB.9 -- IRQ
		;
	state = SPI_NRF2_ReadReg(STATUS);  // 读状态寄存器
	SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state); // 清除TX_DS或MAX_RT中断标志

	                                   // 判断中断类型  
	if (state & MAX_RT)                // 达到重发次数
	{
		SPI_NRF2_WriteReg(FLUSH_TX, NOP);// 清除 TX 寄存器
		return MAX_RT;
	}	
	else if (state & TX_DS)            // 发送完成
		return TX_DS;
	else 
		return ERROR;                    // 其他原因发送失败
}

/** @brief   从 NRF2 的接收缓冲区 -- 读出数据
  * @param   rxbuf: 数组 -- 用于储存将要接收的数据
  * @retval  接收结果， 成功：RX_DR ；失败：ERROR
***/
uint8_t NRF2_Rx_Dat(uint8_t *rxbuf)
{
	uint8_t state;
	NRF2_CE_HIGH();                     // 进入接收状态，开始接收数据包
                            
	while (NRF2_Read_IRQ() != 0)        // 等待接收中断 PB.9 -- IRQ
		;
	NRF2_CE_LOW();                      // 进入待机状态
	state = SPI_NRF2_ReadReg(STATUS);   // 读状态寄存器
	SPI_NRF2_WriteReg(NRF_WRITE_REG + STATUS, state); // 清除中断标志
	                 
	if (state & RX_DR)                  // 判断是否接收到数据    
	{
		SPI_NRF2_ReadBuf(DR_RX_PLOAD, rxbuf, RX_PLOAD_WIDTH); // 读取数据
    SPI_NRF2_WriteReg(FLUSH_RX, NOP); // 清除RX FIFO寄存器		
		
		return RX_DR;                     // 成功接收到数据
	}
	else 
		return ERROR;                     // 接收数据失败
}

/*** @brief  NRF -- 检测函数 ***/
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
	NRF_TX_Mode();					      // 主机发送模式 NRF  - TX_Mode
	NRF2_RX_Mode();					      // 从机接收模式 NRF2 - RX_Mode
		
	status = NRF_Tx_Dat(Tx_Buf);  // 开始发送数据
	switch (status)               // 判断发送状态
	{
		case MAX_RT: printf("\nNRF1: has no Ack, number > MAX_RT --> Faule!");
			break;                    // 无应答，失败
		case ERROR:	printf("\nNRF1:Donot konw why --> Faul!");
			break;                    // 未知原因，失败
		case TX_DS:	printf("\nNRF1:TX and Rx Success!\n");
			break;                    // 发送成功
 	}
		
	status = NRF2_Rx_Dat(Rx_Buf); // 等待接收数据
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

  status = NRF2_Tx_Dat(Tx_Buf);// 开始发送数据
	switch (status)              // 判断发送状态
	{
		case MAX_RT: printf("\nNRF2: has no Ack, number > MAX_RT --> Faule!");
			break;                   // 无应答，失败
		case ERROR:	printf("\nNRF2:Donot konw why --> Faul!");
			break;                   // 未知原因，失败
		case TX_DS:	printf("\nNRF2:TX and Rx Success!\n");
			break;                   // 发送成功
 	}
		
	status = NRF_Rx_Dat(Rx_Buf); // 等待接收数据
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
