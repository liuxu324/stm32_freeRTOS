/********************************************************************************
  * @file    ����HC-SR04
  * @author  liu xu
  * @date    2017-11-07
  ******************************************************************************/ 
#include "bsp_hcsr04.h"
#include "bsp_SysTick.h"
#include "bsp_ds18b20.h"

uint16_t ms_HCSR04_Count; //����

/*** @brief  ����HCSR04_NVIC�ж� ***/
static void HCSR04_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
		
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*** @brief  HCSR04_GPIO��ʼ�� ***/
static void HCSR04_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = HCSR04_Trig; 	// PB.5 -- �������		 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, HCSR04_Trig);

	GPIO_InitStructure.GPIO_Pin = HCSR04_Echo; // PB.6 -- �������� --> PB.1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB, HCSR04_Echo);			
}

/*** @brief  HCSR04_Mode��ʼ�� ***/
static void HCSR04_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;				
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); 	//����ʱ��

	TIM_TimeBaseStructure.TIM_Period = (1000-1); 			//��װֵ -- 1ms
	TIM_TimeBaseStructure.TIM_Prescaler = (72-1); 			//����Ƶ�� -- 1us	
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;	//���ϼ���
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); 

	TIM_ClearFlag(TIM3, TIM_FLAG_Update);  					//��������ж�
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE) ;   			/* ʹ�ܶ�ʱ�������ж� */

	HCSR04_NVIC();											//����Ƕ���жϺ���	

	TIM_Cmd(TIM3, DISABLE);    
}

/*** @brief  HCSR04��ʼ�� ***/
void HCSR04_Init(void)
{
	HCSR04_GPIO_Config();   
	HCSR04_Mode_Config();	
}

/*** @brief  HCSR04--�򿪶�ʱ�� ***/
static void OpenTimer_HCSR04(void)
{
	TIM_SetCounter(TIM3, 0);
	ms_HCSR04_Count = 0;
	TIM_Cmd(TIM3, ENABLE);    
}

/*** @brief  HCSR04--�رն�ʱ�� ***/
static void CloseTimer_HCSR04(void)
{
	TIM_Cmd(TIM3, DISABLE);    
}

/*** @brief  ��ȡ��ʱ��ʱ��
  *  @retval ��ʱ��ʱ��t  
***/
uint32_t GetTime_Echo(void)
{
	uint32_t t = 0;
	t = ms_HCSR04_Count * 1000;  //����ֵΪ1000 --> ʱ�䣺1us*1000 
	t += TIM_GetCounter(TIM3);   //����ʣ���ʱ�� --> ��us
	TIM3->CNT = 0;               //����������
	Delay_us(50*1000);	
	
	return t;
}

/** @brief  ���
  * @param  ��
  * @retval �⵽�ľ���ֵ
***/
float HCSR04_GetLength(void)
{
	uint32_t t = 0;
	uint8_t i = 0;
	float Length = 0.0;
	float sum = 0.0;
	float T = 0.0;
	
	while (5 != i)
	{
		GPIO_SetBits(GPIOB, HCSR04_Trig);
		Delay_us(20);	
		GPIO_ResetBits(GPIOB, HCSR04_Trig);
		while (0 ==  GPIO_ReadInputDataBit(GPIOB, HCSR04_Echo))
			;

		OpenTimer_HCSR04();
		i += 1;

		while (1 ==  GPIO_ReadInputDataBit(GPIOB, HCSR04_Echo))
			;

		CloseTimer_HCSR04();
		t = GetTime_Echo();
//		Length = (float)t/58.1;  //���¶Ȳ���

		T = DS18B20_Get_Temp();
		Length = (float)t / (20000/(331.5+0.607*T));  //���¶Ȳ���
		sum += Length;
	}
	
	Length = sum/5.0;
	return Length;
}

/**** note: ************************
		 2*L = t*344
		 L(m) = t(s)/0.0058
  	 L(cm)*10^-2 = t(us)*10^-6/0.00581
     L(cm) = t(us)/58.1
****/

/*********************************************END OF FILE**********************/
