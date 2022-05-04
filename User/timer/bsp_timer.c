/*********************************************************************************
  * @file    基本定时器的使用 -- TIM6
  * @author  liu xu
  * @version V1.0
  * @date    2020-6-27
  * @brief   基本定时器控制LED定时闪烁
  ******************************************************************************/ 
#include "bsp_timer.h"

/*** @brief  配置TIM NVIC中断 ***/
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
  * @brief  TIM6模式配置
  * @param  无
  * @retval 无
  */
static void TIM6_Mode_Config(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);

	TIM_TimeBaseStructure.TIM_Period = (10000 - 1);			//ARR重装值 -- 100us*N = n(ms) = 1000ms = 1s
	//TIM_TimeBaseStructure.TIM_Period = (10 - 1);			//ARR重装值 -- 100us*N = 100*10(us) = 1ms
	TIM_TimeBaseStructure.TIM_Prescaler = (7200 - 1);		//PSC计数频率 -- 100us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;	//选择时钟不分割
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM6, & TIM_TimeBaseStructure); 

	TIM_ClearFlag(TIM6, TIM_FLAG_Update);					//清除更新中断
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE); 				//使能定时器更新中断

	TIM6_NVIC();											//配置嵌套中断函数
	
	TIM_Cmd(TIM6,ENABLE);
}

/**
 * @brief  定时器6初始化
 * @param  无
 * @retval 无
 */
void TIM6_Init(void)
{
	TIM6_Mode_Config();
}

/*********************************************END OF FILE**********************/
