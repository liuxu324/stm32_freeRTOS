/********************************************************************************
  * @file    调试 DHT11
  * @author  liu xu
  * @date    2017-11-26
  ******************************************************************************/ 
#include "bsp_dht11.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

DHT11_Data_TypeDef DHT11_Data; // 声明存储 DHT11 数据的结构体 

/*** @brief  DHT11_GPIO初始化 ***/
void DHT11_GPIO_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DHT11_CLK, ENABLE);

		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;/* PB.10 -- 通用推挽输出 */		  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
		
	  DHT11_DATA_OUT(HIGH);	// 先置位，空闲模式
}

/*** @brief  设置主机模式 -- 通用推挽输出 ***/
static void DHT11_Mode_Out_PP(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
	  DHT11_DATA_OUT(HIGH);	// 先置位，空闲模式	
}

/*** @brief  设置主机模式 -- 上拉输入***/
static void DHT11_Mode_IPU(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
			
		GPIO_InitStructure.GPIO_Pin = DHT11_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  
		GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
	  DHT11_DATA_OUT(HIGH);	// 先置位，空闲模式	
}

/** @brief  从 DHT11 读取一个字节 -- MSB 先行
		@retval DHT11:以26~28us的高电平表示“0”，以70us的高电平表示“1”。
***/
static uint8_t DHT11_Read_Byte(void)	
{
	uint8_t i, temp = 0;
	
	for (i=0; i<8; i++)
	{
		while (!DHT11_DATA_IN())           // 每bit以50us低电平开始，轮询至50us低电平结束
		  ;
		Delay_us(40);
		if (DHT11_DATA_IN())               // 40us后仍为高电平，表示数据‘1’
		{
			while (DHT11_DATA_IN())          // 等待数据‘1’的高电平结束
			temp |= (uint8_t)(0x01<<(7-i));	 // 把第7-i位置1，MSB先行
		}
		else
		{
			temp &= (uint8_t)~(0x01<<(7-i)); // 把第7-i位置0，MSB先行
		}
	}
	return temp;
}

/** @brief  获取 DHT11:一次完整的数据传输为40bit，高位先出
  * @retval 8bit湿度整数 + 8bit湿度小数 + 8bit温度整数 + 8bit温度小数 + 8bit校验和
***/
static uint8_t Read_DHT11(DHT11_Data_TypeDef *DHT11_Data)
{	
  DHT11_Mode_Out_PP();  // 主机 -- 推挽输出			
  DHT11_DATA_OUT(LOW);				
	Delay_us(18000);      // 18ms
	DHT11_DATA_OUT(HIGH);		
	Delay_us(30);
		
	DHT11_Mode_IPU();	    // 主机 -- 上拉输入，输入模式，判断是否有从机的回响信号		
	if (!DHT11_DATA_IN()) // 判断是否有从机的低电平回响信号
	{
		while (!DHT11_DATA_IN()) // 轮询，直到从机发出的80us低电平信号结束
			;
		while (DHT11_DATA_IN())  // 轮询，直到从机发出的80us高电平信号结束
			;                      // 开始接收数据     
		DHT11_Data->humi_int = DHT11_Read_Byte();  // 湿度的整数部分
		DHT11_Data->humi_deci = DHT11_Read_Byte(); // 湿度的小数部分 
		DHT11_Data->temp_int = DHT11_Read_Byte();  // 温度的整数部分
	  DHT11_Data->temp_deci = DHT11_Read_Byte(); // 温度的小数部分
	  DHT11_Data->check_sum = DHT11_Read_Byte(); // 校验和
		
		DHT11_Mode_Out_PP();     // 主机 -- 推挽输出，数据读取结束，改为输出模式			
		DHT11_DATA_OUT(HIGH);			
		                         // 检查读取的数据是否正确
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

/** @brief  显示 DHT11 获取的温湿度数据
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
