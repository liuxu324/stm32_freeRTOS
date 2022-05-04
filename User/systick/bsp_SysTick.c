/*********************************************************************************/ 
#include "bsp_SysTick.h"

static __IO uint32_t TimingDelay;
 
 /**
  * @brief  启动系统滴答定时器 SysTick(1us 中断一次)
  * @param  无
  * @retval 无
  */
void SysTick_Init(void)
{
    if (SysTick_Config(SystemCoreClock / 1000000))  //对这里做了修改
    {
         while(1);
    }
	/*关闭抵滴答定时器*/
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

/**
  * @brief  延时函数(1us 一个单位)
  */
void Delay_us(__IO uint32_t nTime)
{
    TimingDelay = nTime;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while(TimingDelay != 0);
}

/**
  * @brief  启动系统定时器(1us 中断一次)
  * @param  无
  * @retval 无
  */
void TimingDelay_Decrement(void)
{
    if (TimingDelay != 0x00)
    {
         TimingDelay--;
    }
}

/*********************************************END OF FILE**********************/
