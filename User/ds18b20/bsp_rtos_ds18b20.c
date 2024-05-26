/********************************************************************************
  * @file    调试rtos下DS18B20 注：主要修改us延时函数
  * @author  liu xu
  * @date    2023-02-05
  ******************************************************************************/ 
#include "bsp_rtos_ds18b20.h"
#include "bsp_dwt.h"
#include <stdio.h>

static uint8_t ds18b20_id[8] = {0x28, 0x47, 0x23, 0x10, 0x04, 0x00, 0x00, 0xfe};

/*** @brief  DS18B20_GPIO初始化 ***/
static void DS18B20_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(DS18B20_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;	//PB.10 -- 通用推挽输出	  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
	GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		//先置位
}

/*** @brief  设置主机模式 -- 通用推挽输出 ***/
static void DS18B20_Mode_Out_PP(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
	GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		
}

/*** @brief  设置主机模式 -- 上拉输入***/
static void DS18B20_Mode_IPU(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
		
	GPIO_InitStructure.GPIO_Pin = DS18B20_DQ;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
	GPIO_Init(DS18B20_PORT, &GPIO_InitStructure);
	GPIO_SetBits(DS18B20_PORT, DS18B20_DQ);		
}

/*** @brief  DS18B20_Rst复位 ***/
static void DS18B20_Rst(void)
{
	DS18B20_Mode_Out_PP();	//主机 -- 推挽输出			

	DS18B20_DATA_OUT(LOW);			
	//Delay_us(500);
	dwt_delay_us(500);

	DS18B20_DATA_OUT(HIGH);				
	//Delay_us(15);
	dwt_delay_us(15);
}

/** @brief  DS18B20_检测从机的返回状态
	*	0 -- 成功； 1 -- 失败 ***/
static uint8_t DS18B20_Presence(void)
{
	uint8_t pulse_time = 0;

	DS18B20_Mode_IPU();	//主机 -- 上拉输入		

	while (DS18B20_DATA_IN() && pulse_time<100)
	{
		pulse_time++;
		//Delay_us(1);
		dwt_delay_us(1);
	}
	
	if (pulse_time>=100)	//100us后，存在脉冲还未到来（总线上没有DS18B20）
	{
		return 1;
	}
	else
	{
		pulse_time = 0;
	}
	
	while (!DS18B20_DATA_IN() && pulse_time<240) //存在脉冲（低），且<240us
	{
		pulse_time++;
		//Delay_us(1);
		dwt_delay_us(1);
	}
	
	if (pulse_time>=240)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*** @brief  DS18B20_初始化 ***/
uint8_t DS18B20_rtos_Init(void)
{
	DS18B20_GPIO_Config();   
	DS18B20_Rst();

	return DS18B20_Presence();
}

/** @brief  写字节到DS18B20--低位先行：时间60us<Tx<120us
  * 写0：在[1us,120us]内拉低
	* 写1：在[1us,15us]内拉高
***/
static void DS18B20_Write_Byte(uint8_t dat)	
{
	uint8_t i, testb;
	
	DS18B20_Mode_Out_PP();	//主机 -- 推挽输出			
	for (i=0; i<8; i++)
	{
		testb = dat & 0X01;   //保留1位（写入的值），其余位清0
		dat >>= 1;
		if (testb)            //总时间>60us
		{
			DS18B20_DATA_OUT(LOW);				
			//Delay_us(8);	      //1us<时间<15us
			dwt_delay_us(8);
		
			DS18B20_DATA_OUT(HIGH);				
			//Delay_us(60);
			dwt_delay_us(60);
		}
		else
		{
			DS18B20_DATA_OUT(LOW);				
			//Delay_us(70);	     //60us<Tx<120us
			dwt_delay_us(70);
			
			DS18B20_DATA_OUT(HIGH);				
			//Delay_us(2);       //Tx>1us
			dwt_delay_us(2);
		}
	}
}

/*** @brief  从DS18B20读字节--低位先行 ***/
static uint8_t DS18B20_Read_Byte(void)	
{
	uint8_t i, dat = 0;
	
	for (i=0; i<8; i++)
	{
		dat >>= 1;
		
		DS18B20_Mode_Out_PP();	//主机 -- 推挽输出			
		DS18B20_DATA_OUT(LOW);				
		//Delay_us(6);
		dwt_delay_us(6);
		DS18B20_DATA_OUT(HIGH);		
		//Delay_us(2);
		dwt_delay_us(2);
			
		DS18B20_Mode_IPU();	    //主机 -- 上拉输入		
		if (DS18B20_DATA_IN())
		{
			dat |= 0x80;
		}
		else 
		{
			dat &= 0x7f;
		}
		//Delay_us(8);
		dwt_delay_us(8);
	}
	DS18B20_Mode_Out_PP();	//主机 -- 推挽输出			
	DS18B20_DATA_OUT(HIGH);		

	return dat;
}

/** @brief  获取温度
  * @retval 写/读寄存器 
***/
float DS18B20_rtos_Get_Temp(void)
{
	uint8_t tp_h = 0, tp_l = 0, i = 0, j = 0;
	short s_tem = 0;
	float f_tem = 0.0, sum = 0.0;
	
	while (5 != j)
	{
		DS18B20_Rst();
		DS18B20_Presence();
		DS18B20_Write_Byte(0X55);		//匹配ROM（ds18b20序列号）
		for (i=0; i<8; i++)
		{
			DS18B20_Write_Byte(ds18b20_id[i]);
		}
		DS18B20_Write_Byte(0X44);		//开始温度转换
	
		DS18B20_Rst();
		DS18B20_Presence();
		DS18B20_Write_Byte(0X55);		//匹配ROM（ds18b20序列号）
		for (i=0; i<8; i++)
		{
			DS18B20_Write_Byte(ds18b20_id[i]);
		}
		DS18B20_Write_Byte(0XBE);		//读温度值

		tp_l = DS18B20_Read_Byte();	//读低字节
		tp_h = DS18B20_Read_Byte();	//读高字节	
		s_tem = tp_h << 8;
		s_tem = s_tem | tp_l;
	
		if (s_tem<0)
		{
			f_tem = (~s_tem+1) * 0.0625;//负温度
		}
		else
		{
			f_tem = s_tem * 0.0625;
		}
		printf("f_tem = %.2f\n", f_tem);
		sum += f_tem;
		j++;
	}
	
	printf("sum = %.2f\n", sum);
	return (sum / 5.0);
}

#if 0//test demo
void DS18B20_rtos_test(void)
{
	printf("\nDS18B20-Temperture = %.2f\n", DS18B20_Get_Temp());// 打印温度
}
#endif

/*******************Note*****************/
//		DS18B20_Write_Byte(0XCC);		//跳过ROM
/*********************************************END OF FILE**********************/

