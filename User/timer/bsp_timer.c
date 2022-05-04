/*********************************************************************************
  * @file    ������ʱ����ʹ�� -- TIM6
  * @author  liu xu
  * @version V1.0
  * @date    2020-6-27
  * @brief   ������ʱ������LED��ʱ��˸
  ******************************************************************************/ 
#include "bsp_timer.h"

/*** @brief  ����TIM NVIC�ж� ***/
static void TIM6_NVIC(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  TIM6ģʽ����
  * @param  ��
  * @retval ��
  */
static void TIM6_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

	TIM_TimeBaseStructure.TIM_Period = (10000 - 1);			//ARR��װֵ -- 100us*N = n(ms) = 1000ms = 1s
	//TIM_TimeBaseStructure.TIM_Period = (10 - 1);			//ARR��װֵ -- 100us*N = 100*10(us) = 1ms
	TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);		//PSC����Ƶ�� -- 100us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//ѡ��ʱ�Ӳ��ָ�
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, & TIM_TimeBaseStructure); 

	TIM_ClearFlag(TIM6, TIM_FLAG_Update);					//��������ж�
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); 				//ʹ�ܶ�ʱ�������ж�

	TIM6_NVIC();											//����Ƕ���жϺ���
	
	TIM_Cmd(TIM6,ENABLE);
}

/**
 * @brief  ��ʱ��6��ʼ��
 * @param  ��
 * @retval ��
 */
void TIM6_Init(void)
{
	TIM6_Mode_Config();
}

/*********************************************END OF FILE**********************/
