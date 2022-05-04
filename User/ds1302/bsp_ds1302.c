/********************************************************************************
  * @file    ���� DS1302
  * @author  liu xu
  * @date    2017-11-28
  ******************************************************************************/ 
#include "bsp_ds1302.h"
#include "bsp_SysTick.h"
#include "bsp_usart1.h"

/****************** ����ʱ�����飬����ʼ�� *********************/
uint8_t read_ds1302_time[7] = {0};
uint8_t tran_ds1302_time[7] = {0};

/****************** ����ʱ��ṹ�壬����ʼ�� *********************/
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

/*** @brief  DS1302_GPIO ��ʼ��  ***/
static void DS1302_GPIO_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = DS1302_CE | DS1302_SCK; 			 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // ͨ���������  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
}

/*** @brief  ��������ģʽ -- ͨ��������� ***/
static void DS1302_Mode_OUT(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		RCC_APB2PeriphClockCmd(DS1302_CLK, ENABLE);
	
		GPIO_InitStructure.GPIO_Pin = DS1302_DQ;  
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(DS1302_PORT, &GPIO_InitStructure);
}

/*** @brief  ���ôӻ�ģʽ -- ��������***/
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
* Description    : Read a byte on DS1302(��ȡһ�ֽ����ݣ���λǰ����λ��)
* Input          : cmd (Ҫд��Ŀ����ֽ�)
* Return         : dat (��ȡ������)
*******************************************************************************/
static uint8_t DS1302_ReadByte(uint8_t cmd)
{
    uint8_t i, dat = 0;
    
    DS1302_Mode_OUT();           // DQ ��������Ϊ���         
   
    DS1302_CE_H();               // ��ʼ�����ַ���
		Delay_us(50);	
    DS1302_SCK_L(); 
		Delay_us(50);       
    for (i=0; i<8; i++)          // ��д������֣���ַ�����ӵ�λ��ʼ
    {    
        if(cmd & 0x01)
            DS1302_DQ_H();      
        else
						DS1302_DQ_L();
        DS1302_SCK_H();          // Ӫ��������, д���ݵ� ds1302 
				Delay_us(50);				
        DS1302_SCK_L();
				Delay_us(50);        
        cmd >>= 1;  
    }
   
    DS1302_Mode_IN();            // DQ ��������Ϊ����            
    for (i=0; i<8; i++)
    {
        dat >>= 1;               // �ȶ����������ݵ����λ
        if (DS1302_DATA_IN())    // �ж������ϵĵ�ƽ״̬
           dat |= 0x80;
        else
           dat &= 0x7f;
        DS1302_SCK_H();          
        Delay_us(50);   
        DS1302_SCK_L();          // Ӫ���½���, ��ds1302�����ݶ���      
        Delay_us(50);      
    }  
    DS1302_CE_L();               // �������ݴ���
    
    return dat;
}

/*******************************************************************************
* Function Name  : DS1302_WriteByte
* Description    : Write a byte on DS1302 (��ָ���Ĵ���дһ���ֽ�����)
* Input          : cmd (Ҫд��Ŀ�����) dat (Ҫд�������)
*******************************************************************************/
static void DS1302_WriteByte(uint8_t cmd, uint8_t data)
{
    uint8_t i;
   
    DS1302_Mode_OUT();          // DQ ��������Ϊ���
		DS1302_CE_H();              // ��ʼ�����ַ���
    Delay_us(50);    
  	DS1302_SCK_L(); 
    Delay_us(50);               
    
    for(i=0; i<8; i++)
    {	
        if(cmd & 0x01)          // ��д������֣���ַ�����ӵ�λ��ʼ
          DS1302_DQ_H(); 
        else
          DS1302_DQ_L();      			
				DS1302_SCK_H();         // Ӫ��������, д���ݵ� ds1302              
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
		
    DS1302_CE_L();              // �������ݴ��䣬���͸�λ����
}

/*******************************************************************************
 * Function Name  : DS1302_WriteTime
 * Description    : д���ʼʱ��
 * Input          : TIME_TypeDef *time -- ʱ��ṹ��
*******************************************************************************/
static void DS1302_WriteTime(TIME_TypeDef *time)
{   
		DS1302_WriteByte(WrControl, 0x00);      // �ر�д����������д������
	
  	DS1302_WriteByte(WrSec, time->sec);     
	  DS1302_WriteByte(WrMin, time->min);     
	  DS1302_WriteByte(WrHour, time->hour);   
	  DS1302_WriteByte(WrWeek, time->week);   
	  DS1302_WriteByte(WrDate, time->date);   
  	DS1302_WriteByte(WrMonth, time->month); 
  	DS1302_WriteByte(WrYear, time->year);   // д��year
	
	  DS1302_WriteByte(WrControl, 0x80);      // ����д��������ֹд������
}

/*******************************************************************************
 * Function Name  : DS1302_Init
 * Description    : DS1302��ʼ��
 * Input          : ���� DS1302_WriteTime
*******************************************************************************/
void DS1302_Init(void)
{
  	DS1302_GPIO_Init();
	
  	DS1302_CE_L();
		DS1302_SCK_L();
  	DS1302_WriteTime(rtc_time); // д��ʼ��ʱ��
}

/*******************************************************************************
 * Function Name  : DS1302_ReadTime
 * Description    : ����ʱ��
 * Output         : read_ds1302_time[] -- ��ʱ�䵽���� 
*******************************************************************************/
static void DS1302_ReadTime(void)
{   
    uint8_t i = 0;
  
  	read_ds1302_time[6] = DS1302_ReadByte(RdSec);    // ��Sec
    read_ds1302_time[5] = DS1302_ReadByte(RdMin);    // ��Min
    read_ds1302_time[4] = DS1302_ReadByte(RdHour);   // ��Hour
		read_ds1302_time[3] = DS1302_ReadByte(RdWeek);   // ��Week
    read_ds1302_time[2] = DS1302_ReadByte(RdDate);   // ��Date
	  read_ds1302_time[1] = DS1302_ReadByte(RdMonth);  // ��Month
	  read_ds1302_time[0] = DS1302_ReadByte(RdYear);   // ����year

   	for(i=0; i<7; i++)
    {
       tran_ds1302_time[i] = (read_ds1302_time[i]>>4)*10 + \
			     (read_ds1302_time[i]%16);   // ��BCD��ת����ʮ������ֵ     
    }
}

/*******************************************************************************
 * Function Name  : DS1302_Time_Show
 * Description    : ��ʾ������ʱ�� 
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

/********************** ����д��ʼ��ʱ�� **********************/
/****************** ����ʱ�����飬����ʼ�� *********************/
//uint8_t init_ds1302_time[6] = {0X17, 0X12, 0X08, 0X20, 0X30, 0X00};  
//uint8_t read_ds1302_time[6] = {0};
//uint8_t tran_ds1302_time[6] = {0};
/*******************************************************************************
 * Function Name  : DS1302_Init
 * Description    : DS1302��ʼ��
 * Input          : DS1302_Init_time() -- ����
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
 * Description    : д�붨��ĳ�ʼ��ʱ��
 * Input          : init_ds1302_time[] -- ��ʼ��ʱ������
*******************************************************************************/
//void DS1302_Init_time(void)
//{
//		DS1302_WriteByte(WrControl, 0x00); // �ر�д����������д������

//	  DS1302_WriteByte(WrSec, init_ds1302_time[5]);   // д��Sec
//		DS1302_WriteByte(WrMin, init_ds1302_time[4]);   // д��Min
//		DS1302_WriteByte(WrHour, init_ds1302_time[3]);  // д��Hour
//		DS1302_WriteByte(WrDate, init_ds1302_time[2]);  // д��Date
//		DS1302_WriteByte(WrMonth, init_ds1302_time[1]); // д��Month
//		DS1302_WriteByte(WrYear, init_ds1302_time[0]);  // д��year

//  	DS1302_WriteByte(WrControl, 0x80); // ����д��������ֹд������
//}

/*********************************************END OF FILE**********************/
