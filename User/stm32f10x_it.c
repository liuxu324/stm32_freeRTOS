/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include "bsp_SysTick.h"
#include "bsp_led.h"
/*FreeRTOS使用*/
#include "FreeRTOS.h"
#include "task.h"

extern volatile unsigned char can_flag;

extern uint16_t ms_HCSR04_Count;
extern CanRxMsg RxMessage;

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * This function handles CAN exception.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);

	/* 比较数据和 ID */
	if ((RxMessage.ExtId == 0x1314) && (RxMessage.IDE == CAN_ID_EXT) 
	 && (RxMessage.DLC == 2) && ((RxMessage.Data[1]|RxMessage.Data[0]<<8) == 0xABCD)) 
	{
		can_flag = 0; //接收成功
	} 
	else
	{
		can_flag = 0xff; //接收失败
	}
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
	{
	}
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
#if 0//liuxu FreeRTOS实现
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}
#endif

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.(系统时钟中断)
  * @param  None
  * @retval None
  */
#if 1//liuxu FreeRTOS 定义
extern void xPortSysTickHandler(void);
/*中断服务函数*/
void SysTick_Handler(void)
{
#if (1 == INCLUDE_xTaskGetSchedulerState)
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
	{
#endif /* INCLUDE_xTaskGetSchedulerState */
		
		xPortSysTickHandler();

#if (1 == INCLUDE_xTaskGetSchedulerState)
	}
#endif /* INCLUDE_xTaskGetSchedulerState */
}
#else
void SysTick_Handler(void)
{
	TimingDelay_Decrement();
}
#endif

/**
  * @brief  TIM3 中断：对HCSR04_Echo脉冲计数 -- 1us/次
  * @param  None
  * @retval None
  */
void TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		TIM_ClearFlag(TIM3, TIM_FLAG_Update);
		ms_HCSR04_Count++;
	}
}

/************************* TIM6 ************************/
static uint32_t ms_Count = 0; //计数

/**
  * @brief  TIM 中断：控制LED闪烁 -- 1s/次 
  * @param  None
  * @retval None
  */
void TIM6_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)//1s中断1次
	{
		TIM_ClearFlag(TIM6, TIM_FLAG_Update);
		//LED1_TOGGLE;
		ms_Count++;

		if (ms_Count >= 2)//1s闪烁
		{
			/*重置*/
			ms_Count = 0;
			//LED2_TOGGLE;
		}
	}	
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
