/********************************************************************************
  * @file    调试 DS1302
  * @author  liu xu
  * @date    2017-11-28
  ******************************************************************************/ 
#include "bsp_ds1302.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

/****************** 声明时间数组，并初始化 *********************/
uint8_t read_ds1302_time[7] = {0};
uint8_t tran_ds1302_time[7] = {0};

/****************** 定义时间结构体，并初始化 *********************/
typedef struct
{
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t week;
	uint8_t date;
	uint8_t month;
  uint8_t year;
} TIME_TypeDef;
 
TIME_TypeDef init_ds1302_time = {0X00, 0X58, 0X23, 0X01, 0X11, 0X12, 0X17};  
TIME_TypeDef *rtc_time = &init_ds1302_time;

/*** @brief  DS1302_GPIO 初始化  ***/
static void DS1302_GPIO_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = DS1302_CE | DS1302_SCK; 			 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 通用推挽输出  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
}

/*** @brief  设置主机模式 -- 通用推挽输出 ***/
static void DS1302_Mode_OUT(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = DS1302_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
}

/*** @brief  设置从机模式 -- 浮空输入***/
static void DS1302_Mode_IN(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = DS1302_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // IN_FLOATING  
		GPIO_Init(DS1302_PORT, &GPIO_InitStructure);		
}

/*******************************************************************************
* Function Name  : DS1302_ReadByte
* Description    : Read a byte on DS1302(读取一字节数据，低位前、高位后)
* Input          : cmd (要写入的控制字节)
* Return         : dat (读取的数据)
*******************************************************************************/
static uint8_t DS1302_ReadByte(uint8_t cmd)
{
    uint8_t i, dat = 0;
    
    DS1302_Mode_OUT();           // DQ 方向设置为输出         
   
    DS1302_CE_H();               // 开始控制字访问
		Delay_us(50);	
    DS1302_SCK_L(); 
		Delay_us(50);       
    for (i=0; i<8; i++)          // 先写入控制字（地址），从低位开始
    {    
        if(cmd & 0x01)
            DS1302_DQ_H();      
        else
						DS1302_DQ_L();
        DS1302_SCK_H();          // 营造上升沿, 写数据到 ds1302 
				Delay_us(50);				
        DS1302_SCK_L();
				Delay_us(50);        
        cmd >>= 1;  
    }
   
    DS1302_Mode_IN();            // DQ 方向设置为输入            
    for (i=0; i<8; i++)
    {
        dat >>= 1;               // 先读出的是数据的最低位
        if (DS1302_DATA_IN())    // 判断总线上的电平状态
           dat |= 0x80;
        else
           dat &= 0x7f;
        DS1302_SCK_H();          
        Delay_us(50);   
        DS1302_SCK_L();          // 营造下降沿, 将ds1302的数据读出      
        Delay_us(50);      
    }  
    DS1302_CE_L();               // 结束数据传输
    
    return dat;
}

/*******************************************************************************
* Function Name  : DS1302_WriteByte
* Description    : Write a byte on DS1302 (向指定寄存器写一个字节数据)
* Input          : cmd (要写入的控制字) dat (要写入的数据)
*******************************************************************************/
static void DS1302_WriteByte(uint8_t cmd, uint8_t data)
{
    uint8_t i;
   
    DS1302_Mode_OUT();          // DQ 方向设置为输出
		DS1302_CE_H();              // 开始控制字访问
    Delay_us(50);    
  	DS1302_SCK_L(); 
    Delay_us(50);               
    
    for(i=0; i<8; i++)
    {	
        if(cmd & 0x01)          // 先写入控制字（地址），从低位开始
          DS1302_DQ_H(); 
        else
          DS1302_DQ_L();      			
				DS1302_SCK_H();         // 营造上升沿, 写数据到 ds1302              
        Delay_us(50);        
				DS1302_SCK_L();
        Delay_us(50);				
        cmd >>= 1;
    }
    
    for(i=0; i<8; i++)
    {   
        if(data & 0x01)
          DS1302_DQ_H(); 
        else
          DS1302_DQ_L();      
        DS1302_SCK_H();
        Delay_us(50);				
        DS1302_SCK_L();
        Delay_us(50);				
        data >>= 1;   
    }   
		
    DS1302_CE_L();              // 结束数据传输，拉低复位引脚
}

/*******************************************************************************
 * Function Name  : DS1302_WriteTime
 * Description    : 写入初始时间
 * Input          : TIME_TypeDef *time -- 时间结构体
*******************************************************************************/
static void DS1302_WriteTime(TIME_TypeDef *time)
{   
		DS1302_WriteByte(WrControl, 0x00);      // 关闭写保护，可以写入数据
	
  	DS1302_WriteByte(WrSec, time->sec);     
	  DS1302_WriteByte(WrMin, time->min);     
	  DS1302_WriteByte(WrHour, time->hour);   
	  DS1302_WriteByte(WrWeek, time->week);   
	  DS1302_WriteByte(WrDate, time->date);   
  	DS1302_WriteByte(WrMonth, time->month); 
  	DS1302_WriteByte(WrYear, time->year);   // 写入year
	
	  DS1302_WriteByte(WrControl, 0x80);      // 开启写保护，禁止写入数据
}

/*******************************************************************************
 * Function Name  : DS1302_Init
 * Description    : DS1302初始化
 * Input          : 调用 DS1302_WriteTime
*******************************************************************************/
void DS1302_Init(void)
{
  	DS1302_GPIO_Init();
	
  	DS1302_CE_L();
		DS1302_SCK_L();
  	DS1302_WriteTime(rtc_time); // 写初始化时间
}

/*******************************************************************************
 * Function Name  : DS1302_ReadTime
 * Description    : 读出时间
 * Output         : read_ds1302_time[] -- 读时间到数组 
*******************************************************************************/
static void DS1302_ReadTime(void)
{   
    uint8_t i = 0;
  
  	read_ds1302_time[6] = DS1302_ReadByte(RdSec);    // 读Sec
    read_ds1302_time[5] = DS1302_ReadByte(RdMin);    // 读Min
    read_ds1302_time[4] = DS1302_ReadByte(RdHour);   // 读Hour
		read_ds1302_time[3] = DS1302_ReadByte(RdWeek);   // 读Week
    read_ds1302_time[2] = DS1302_ReadByte(RdDate);   // 读Date
	  read_ds1302_time[1] = DS1302_ReadByte(RdMonth);  // 读Month
	  read_ds1302_time[0] = DS1302_ReadByte(RdYear);   // 读出year

   	for(i=0; i<7; i++)
    {
       tran_ds1302_time[i] = (read_ds1302_time[i]>>4)*10 + \
			     (read_ds1302_time[i]%16);   // 将BCD码转换成十进制数值     
    }
}

/*******************************************************************************
 * Function Name  : DS1302_Time_Show
 * Description    : 显示读出的时间 
 * Output         : Date:2017.12.11  Week:1  Time:20:30:24 
*******************************************************************************/
void DS1302_Time_Show(void)
{
    DS1302_ReadTime();
      
    printf("\nDate:%4d.%2d.%2d  ", 2000+tran_ds1302_time[0], \
	        tran_ds1302_time[1], tran_ds1302_time[2]); 
	
printf("Week:%d  ", tran_ds1302_time[3]);    
  
  printf("Time:%2d:%2d:%2d \n", tran_ds1302_time[4], \
	        tran_ds1302_time[5], tran_ds1302_time[6]);    
}

/********************** 数组写初始化时间 **********************/
/****************** 声明时间数组，并初始化 *********************/
//uint8_t init_ds1302_time[6] = {0X17, 0X12, 0X08, 0X20, 0X30, 0X00};  
//uint8_t read_ds1302_time[6] = {0};
//uint8_t tran_ds1302_time[6] = {0};
/*******************************************************************************
 * Function Name  : DS1302_Init
 * Description    : DS1302初始化
 * Input          : DS1302_Init_time() -- 数组
*******************************************************************************/
//void DS1302_Init(void)
//{
//  	DS1302_GPIO_Init();
//	
//  	DS1302_CE_L();
//		DS1302_SCK_L();
//		DS1302_Init_time();
//}
/*******************************************************************************
 * Function Name  : DS1302_Init_time
 * Description    : 写入定义的初始化时间
 * Input          : init_ds1302_time[] -- 初始化时间数组
*******************************************************************************/
//void DS1302_Init_time(void)
//{
//		DS1302_WriteByte(WrControl, 0x00); // 关闭写保护，可以写入数据

//	  DS1302_WriteByte(WrSec, init_ds1302_time[5]);   // 写入Sec
//		DS1302_WriteByte(WrMin, init_ds1302_time[4]);   // 写入Min
//		DS1302_WriteByte(WrHour, init_ds1302_time[3]);  // 写入Hour
//		DS1302_WriteByte(WrDate, init_ds1302_time[2]);  // 写入Date
//		DS1302_WriteByte(WrMonth, init_ds1302_time[1]); // 写入Month
//		DS1302_WriteByte(WrYear, init_ds1302_time[0]);  // 写入year

//  	DS1302_WriteByte(WrControl, 0x80); // 开启写保护，禁止写入数据
//}

/*********************************************END OF FILE**********************/
