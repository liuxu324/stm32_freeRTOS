/********************************************************************************
  * @file    ���� DHT11
  * @author  liu xu
  * @date    2017-11-26
  ******************************************************************************/ 
#include "bsp_dht11.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

DHT11_Data_TypeDef DHT11_Data; // �����洢 DHT11 ���ݵĽṹ�� 

/*** @brief  DHT11_GPIO��ʼ�� ***/
void DHT11_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DHT11_CLK, ENABLE);

		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;/* PB.10 -- ͨ��������� */		  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
		
	  DHT11_DATA_OUT(HIGH);	// ����λ������ģʽ
}

/*** @brief  ��������ģʽ -- ͨ��������� ***/
static void DHT11_Mode_Out_PP(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
	  DHT11_DATA_OUT(HIGH);	// ����λ������ģʽ	
}

/*** @brief  ��������ģʽ -- ��������***/
static void DHT11_Mode_IPU(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
	  DHT11_DATA_OUT(HIGH);	// ����λ������ģʽ	
}

/** @brief  �� DHT11 ��ȡһ���ֽ� -- MSB ����
		@retval DHT11:��26~28us�ĸߵ�ƽ��ʾ��0������70us�ĸߵ�ƽ��ʾ��1����
***/
static uint8_t DHT11_Read_Byte(void)	
{
	uint8_t i, temp = 0;
	
	for (i=0; i<8; i++)
	{
		while (!DHT11_DATA_IN())           // ÿbit��50us�͵�ƽ��ʼ����ѯ��50us�͵�ƽ����
		  ;
		Delay_us(40);
		if (DHT11_DATA_IN())               // 40us����Ϊ�ߵ�ƽ����ʾ���ݡ�1��
		{
			while (DHT11_DATA_IN())          // �ȴ����ݡ�1���ĸߵ�ƽ����
			temp |= (uint8_t)(0x01<<(7-i));	 // �ѵ�7-iλ��1��MSB����
		}
		else
		{
			temp &= (uint8_t)~(0x01<<(7-i)); // �ѵ�7-iλ��0��MSB����
		}
	}
	return temp;
}

/** @brief  ��ȡ DHT11:һ�����������ݴ���Ϊ40bit����λ�ȳ�
  * @retval 8bitʪ������ + 8bitʪ��С�� + 8bit�¶����� + 8bit�¶�С�� + 8bitУ���
***/
static uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{	
  DHT11_Mode_Out_PP();  // ���� -- �������			
  DHT11_DATA_OUT(LOW);				
	Delay_us(18000);      // 18ms
	DHT11_DATA_OUT(HIGH);		
	Delay_us(30);
		
	DHT11_Mode_IPU();	    // ���� -- �������룬����ģʽ���ж��Ƿ��дӻ��Ļ����ź�		
	if (!DHT11_DATA_IN()) // �ж��Ƿ��дӻ��ĵ͵�ƽ�����ź�
	{
		while (!DHT11_DATA_IN()) // ��ѯ��ֱ���ӻ�������80us�͵�ƽ�źŽ���
			;
		while (DHT11_DATA_IN())  // ��ѯ��ֱ���ӻ�������80us�ߵ�ƽ�źŽ���
			;                      // ��ʼ��������     
		DHT11_Data->humi_int = DHT11_Read_Byte();  // ʪ�ȵ���������
		DHT11_Data->humi_deci = DHT11_Read_Byte(); // ʪ�ȵ�С������ 
		DHT11_Data->temp_int = DHT11_Read_Byte();  // �¶ȵ���������
	  DHT11_Data->temp_deci = DHT11_Read_Byte(); // �¶ȵ�С������
	  DHT11_Data->check_sum = DHT11_Read_Byte(); // У���
		
		DHT11_Mode_Out_PP();     // ���� -- ������������ݶ�ȡ��������Ϊ���ģʽ			
		DHT11_DATA_OUT(HIGH);			
		                         // ����ȡ�������Ƿ���ȷ
		if (DHT11_Data->check_sum == DHT11_Data->humi_int + DHT11_Data->humi_deci + \
			  DHT11_Data->temp_int + DHT11_Data->temp_deci)
			return SUCCESS;
		else
			return ERROR;
	}
	else
	{
		return ERROR;
	}
}

/** @brief  ��ʾ DHT11 ��ȡ����ʪ������
***/
void Show_DHT11(void)
{
		if (Read_DHT11(&DHT11_Data) == SUCCESS)
		{
			printf("\n\nDHT11: Humi = %d.%d%%,  Temp = %d.%d\n", DHT11_Data.humi_int, \
			  DHT11_Data.humi_deci, DHT11_Data.temp_int, DHT11_Data.temp_deci);
		}
		else
				printf("\n\nDHT11 -- ERROR!\n");
}
/*******************Note*****************/
/*********************************************END OF FILE**********************/
