/********************************************************************************
  * @file    调试hy2615
  * @author  liu xu
  * @date    2019-8-4
  ******************************************************************************/ 
#include "bsp_usart1.h"
#include "bsp_SysTick.h"
#include "bsp_hy2615.h"

/*** @brief  IIC1 I/O配置 ***/
static void I2C_GPIO_Config(void)
{
	// GPIO 结构体
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// GPIO 初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

	GPIO_InitStructure.GPIO_Pin = IIC1_SCK | IIC1_SDA; 			   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; // 集电极开漏输出
	GPIO_Init(IIC1_PORT, &GPIO_InitStructure);
}

/*** @brief  IIC 工作模式配置 ***/
static void I2C_Mode_Config(void)
{
	// IIC 结构体
	I2C_InitTypeDef I2C_InitStructure;
	
	// IIC配置
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C; 			/* I2C 配置 */
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = STM32_ADD;      // 主机IIC地址 0x0A
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /* I2C 寻址模式 */
	I2C_InitStructure.I2C_ClockSpeed = I2C_Speed; 		// 通信速率
	I2C_Init(I2C1, &I2C_InitStructure);
	I2C_Cmd(I2C1, ENABLE);
}

/*** @brief  HY2615 初始化 (IIC总线) ***/
void HY2615_Init(void)
{
	I2C_GPIO_Config();
	I2C_Mode_Config();
}

/*******************************************************************************
 * Function Name  : HY2615_ReadVersion
 * Description    : IIC test 
 * Input          : cmd_add
 * Return         : Version -- HY_BP1.0
*******************************************************************************/
static void HY2615_ReadVersion(u8* pBuffer, u8 ReadAddr, u16 NumByteToRead)
{
	while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
	{
		;
	}
	
	/* Send START condition */
	I2C_GenerateSTART(I2C1, ENABLE);
	
	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	{	
		;
	}

	/* Send EEPROM address for write */
	//I2C_Send7bitAddress(I2C1, HY2615_ADD, I2C_Direction_Transmitter);
	/* Send EEPROM address for read */
	I2C_Send7bitAddress(I2C1, HY2615_ADD, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */ 
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
		;

	/* Clear EV6 by setting again the PE bit */
	I2C_Cmd(I2C1, ENABLE);

	/* Send the EEPROM's internal address to write to */
	I2C_SendData(I2C1, ReadAddr);

	/* Test on EV8 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		;

	/* Send STRAT condition a second time */
	I2C_GenerateSTART(I2C1, ENABLE);

	/* Test on EV5 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
		;
	/* Send EEPROM address for read */
	I2C_Send7bitAddress(I2C1, HY2615_ADD, I2C_Direction_Receiver);

	/* Test on EV6 and clear it */
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
		;
	
	/* While there is data to be read */
	while (NumByteToRead) 
	{
		if (NumByteToRead == 1) 
		{
			/* Disable Acknowledgement */
			I2C_AcknowledgeConfig(I2C1, DISABLE);
			
			/* Send STOP Condition */
			I2C_GenerateSTOP(I2C1, ENABLE);
		}
	
		/* Test on EV7 and clear it */
		if (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED)) 
		{
			/* Read a byte from the EEPROM */
			*pBuffer = I2C_ReceiveData(I2C1);
					
			/* Point to the next location where the byte read will be saved */
			pBuffer++;

			/* Decrement the read bytes counter */
			NumByteToRead--;
		}
	}

	/* Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(I2C1, ENABLE);
}

/*******************************************************************************
 * Function Name  : HY2615_I2C_Test
 * Description    : IIC test 
 * Input          : cmd_add
 * Return         : Version -- HY_BP1.0
*******************************************************************************/
void HY2615_I2C_Test(void)
{
	uint8_t g_data[32] = {0};	// Data
	
	HY2615_ReadVersion(g_data, CMD_ADD0, 32);
	printf("\n*** Rec:%s\n", g_data);
}
/*********************************************END OF FILE**********************/
