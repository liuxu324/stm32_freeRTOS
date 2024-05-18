/********************************************************************************
  * @file    dwt rtos下实现us延时
  * @author  liu xu
  * @date    2023-02-05
  ******************************************************************************/ 
#include "bsp_dwt.h"

#define DWT_CYCCNT	(*(__IO uint32_t *)0xE0001004) // 显示或设置处理器的周期计数值
#define DWT_CTRL    (*(__IO uint32_t *)0xE0001000)
#define DEM_CR      (*(__IO uint32_t *)0xE000EDFC)
#define DBGMCU_CR   (*(__IO uint32_t *)0xE0042004)	//MCU调试模块控制寄存器，详细内容参考《stm32中文参考手册》调试支持（DBG）章节，747页

#define DEM_CR_TRCENA		(0x01 << 24)	// DEMCR的DWT使能位
#define DWT_CTRL_CYCCNTENA	(0x01 << 0)		// DWT的SYCCNT使能位

void dwt_delay_init(void)
{
	//使能DWT外设
	DEM_CR |= (uint32_t)DEM_CR_TRCENA;

	//DWT CYCCNT寄存器计数清0
	DWT_CYCCNT = (uint32_t)0u;

	//使能Cortex-M3 DWT CYCCNT寄存器
	DWT_CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA;
}

// 微秒延时
#if 1
void dwt_delay_us(uint32_t uSec)
{
	uint32_t tStart, tCnt, tDelayCnt;
		   
	tStart = DWT_CYCCNT;									/* 刚进入时的计数器值 */
	tCnt = 0;
	tDelayCnt = uSec * (SystemCoreClock / 1000000); 		/* 需要的节拍数 */					  

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* 求减过程中，如果发生第一次32位计数器重新计数，依然可以正确计算 */		 
	}
}
#else
void dwt_delay_us(uint32_t uSec)
{
    uint32_t ticks_start, ticks_end, ticks_delay;
	
	if(uSec > 10000) 
	{
		uSec = 10000;
	}
	
    ticks_start = DWT_CYCCNT;
    ticks_delay = (uSec * (SystemCoreClock / (1000000))); // 将微秒数换算成滴答数
    ticks_end = ticks_start + ticks_delay;

    // ticks_end没有溢出
    if (ticks_end >= ticks_start)
    {
        // DWT_CYCCNT在上述计算的这段时间中没有溢出
        if(DWT_CYCCNT > ticks_start)
        {
            while (DWT_CYCCNT < ticks_end);
        }
        // DWT_CYCCNT溢出
        else
        {
            // 已经超时，直接退出
            return;
        }
    }
    else // ticks_end溢出
    {
        // DWT_CYCCNT在上述计算的这段时间中没有溢出
        if(DWT_CYCCNT > ticks_start)
        {
            // 等待DWT_CYCCNT的值溢出
            while( DWT_CYCCNT > ticks_end );
        }
        // 等待溢出后的DWT_CYCCNT到达ticks_end
        while( DWT_CYCCNT < ticks_end );
    }
}
#endif

void dwt_delay_ms(uint16_t ms)
{
	uint16_t i;

	for(i = 0; i < ms; i++)
    {
        // delay 1ms
        dwt_delay_us(1*1000);
    }
}

/*********************************************END OF FILE**********************/

