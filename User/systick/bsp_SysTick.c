/*********************************************************************************/ 
#include "bsp_SysTick.h"

static __IO uint32_t TimingDelay;
 
 /**
  * @brief  ����ϵͳ�δ�ʱ�� SysTick(1us �ж�һ��)
  * @param  ��
  * @retval ��
  */
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000000))  //�����������޸�
    {
         while(1);
    }
	/*�رյֵδ�ʱ��*/
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  ��ʱ����(1us һ����λ)
  */
void Delay_us(__IO uint32_t nTime)
{
    TimingDelay = nTime;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(TimingDelay != 0);
}

/**
  * @brief  ����ϵͳ��ʱ��(1us �ж�һ��)
  * @param  ��
  * @retval ��
  */
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    {
         TimingDelay--;
    }
}

/*********************************************END OF FILE**********************/
