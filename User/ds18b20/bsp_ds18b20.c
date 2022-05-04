/********************************************************************************
  * @file    ����DS18B20
  * @author  liu xu
  * @date    2017-11-12
  ******************************************************************************/ 
#include "bsp_ds18b20.h"
#include "bsp_SysTick.h"

static uint8_t ds18b20_id[8] = {0x28, 0x47, 0x23, 0x10, 0x04, 0x00, 0x00, 0xfe};

/*** @brief  DS18B20_GPIO��ʼ�� ***/
static void DS18B20_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS18B20_CLK, ENABLE);

		GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;/* PB.10 -- ͨ��������� */		  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
		GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		//����λ����ߣ�
}

/*** @brief  ��������ģʽ -- ͨ��������� ***/
static void DS18B20_Mode_Out_PP(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
		GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		
}

/*** @brief  ��������ģʽ -- ��������***/
static void DS18B20_Mode_IPU(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
		GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
		GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		
}

/*** @brief  DS18B20_Rst��λ ***/
static void DS18B20_Rst(void)
{
		DS18B20_Mode_Out_PP();	//���� -- �������			

		DS18B20_DATA_OUT(LOW);				
		Delay_us(500);	
	
		DS18B20_DATA_OUT(HIGH);				
		Delay_us(15);
}

/** @brief  DS18B20_���ӻ��ķ���״̬
	*	0 -- �ɹ��� 1 -- ʧ�� ***/
static uint8_t DS18B20_Presence(void)
{
		uint8_t pulse_time = 0;
	
		DS18B20_Mode_IPU();	//���� -- ��������		
		while (DS18B20_DATA_IN() && pulse_time<100)
		{
				pulse_time++;
				Delay_us(1);		
		}
		if (pulse_time>=100)	//100us�󣬴������廹δ������������û��DS18B20��
			return 1;
		else
			pulse_time = 0;
		
		while (!DS18B20_DATA_IN() && pulse_time<240) //�������壨�ͣ�����<240us
		{
				pulse_time++;
				Delay_us(1);		
		}
		if (pulse_time>=240)
			return 1;
		else
			return 0;
}

/*** @brief  DS18B20_��ʼ�� ***/
uint8_t DS18B20_Init(void)
{
	DS18B20_GPIO_Config();   
	DS18B20_Rst();

	return DS18B20_Presence();	
}

/** @brief  д�ֽڵ�DS18B20--��λ���У�ʱ��60us<Tx<120us
  * д0����[1us,120us]������
	* д1����[1us,15us]������
***/
void DS18B20_Write_Byte(uint8_t dat)	
{
	uint8_t i, testb;
	
	DS18B20_Mode_Out_PP();	//���� -- �������			
	for (i=0; i<8; i++)
	{
		testb = dat & 0X01;   //����1λ��д���ֵ��������λ��0
		dat >>= 1;
		if (testb)            //��ʱ��>60us
		{
			DS18B20_DATA_OUT(LOW);				
			Delay_us(8);	      //1us<ʱ��<15us
	
			DS18B20_DATA_OUT(HIGH);				
			Delay_us(60);
		}
		else
		{
			DS18B20_DATA_OUT(LOW);				
			Delay_us(70);	     //60us<Tx<120us
	
			DS18B20_DATA_OUT(HIGH);				
			Delay_us(2);       //Tx>1us
		}
	}
}

/*** @brief  ��DS18B20���ֽ�--��λ���� ***/
uint8_t DS18B20_Read_Byte(void)	
{
	uint8_t i, dat = 0;
	
	for (i=0; i<8; i++)
	{
		dat >>= 1;
		
		DS18B20_Mode_Out_PP();	//���� -- �������			
		DS18B20_DATA_OUT(LOW);				
		Delay_us(6);
		DS18B20_DATA_OUT(HIGH);		
		Delay_us(2);
		
		DS18B20_Mode_IPU();	    //���� -- ��������		
		if (DS18B20_DATA_IN())
			dat |= 0x80;
		else 
			dat &= 0x7f;
		Delay_us(8);
	}
	DS18B20_Mode_Out_PP();	//���� -- �������			
	DS18B20_DATA_OUT(HIGH);		

	return dat;
}

/** @brief  ��ȡ�¶�
  * @retval д/���Ĵ��� 
***/
float DS18B20_Get_Temp(void)
{
	uint8_t tp_h = 0, tp_l = 0, i = 0, j = 0;
	short s_tem = 0;
	float f_tem = 0.0, sum = 0.0 ;
	
	while (5 != j)
	{
		DS18B20_Rst();
		DS18B20_Presence();
		DS18B20_Write_Byte(0X55);		//ƥ��ROM��ds18b20���кţ�
		for (i=0; i<8; i++)
		{
			DS18B20_Write_Byte(ds18b20_id[i]);
		}
		DS18B20_Write_Byte(0X44);		//��ʼ�¶�ת��
	
		DS18B20_Rst();
		DS18B20_Presence();
		DS18B20_Write_Byte(0X55);		//ƥ��ROM��ds18b20���кţ�
		for (i=0; i<8; i++)
		{
			DS18B20_Write_Byte(ds18b20_id[i]);
		}
		DS18B20_Write_Byte(0XBE);		//���¶�ֵ

		tp_l = DS18B20_Read_Byte();	//�����ֽ�
		tp_h = DS18B20_Read_Byte();	//�����ֽ�	
		s_tem = tp_h << 8;
		s_tem = s_tem | tp_l;
	
		if (s_tem<0)	f_tem = (~s_tem+1) * 0.0625;//���¶�
		else	f_tem = s_tem * 0.0625;

		sum += f_tem;
		j++;
	}
	return (sum / 5.0);
}
/*******************Note*****************/
//		DS18B20_Write_Byte(0XCC);		//����ROM
/*********************************************END OF FILE**********************/
