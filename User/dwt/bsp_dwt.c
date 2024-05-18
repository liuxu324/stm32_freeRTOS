/********************************************************************************
  * @file    dwt rtos��ʵ��us��ʱ
  * @author  liu xu
  * @date    2023-02-05
  ******************************************************************************/ 
#include "bsp_dwt.h"

#define DWT_CYCCNT	(*(__IO uint32_t *)0xE0001004) // ��ʾ�����ô����������ڼ���ֵ
#define DWT_CTRL    (*(__IO uint32_t *)0xE0001000)
#define DEM_CR      (*(__IO uint32_t *)0xE000EDFC)
#define DBGMCU_CR   (*(__IO uint32_t *)0xE0042004)	//MCU����ģ����ƼĴ�������ϸ���ݲο���stm32���Ĳο��ֲᡷ����֧�֣�DBG���½ڣ�747ҳ

#define DEM_CR_TRCENA		(0x01 << 24)	// DEMCR��DWTʹ��λ
#define DWT_CTRL_CYCCNTENA	(0x01 << 0)		// DWT��SYCCNTʹ��λ

void dwt_delay_init(void)
{
	//ʹ��DWT����
	DEM_CR |= (uint32_t)DEM_CR_TRCENA;

	//DWT CYCCNT�Ĵ���������0
	DWT_CYCCNT = (uint32_t)0u;

	//ʹ��Cortex-M3 DWT CYCCNT�Ĵ���
	DWT_CTRL |= (uint32_t)DWT_CTRL_CYCCNTENA;
}

// ΢����ʱ
#if 1
void dwt_delay_us(uint32_t uSec)
{
	uint32_t tStart, tCnt, tDelayCnt;
		   
	tStart = DWT_CYCCNT;									/* �ս���ʱ�ļ�����ֵ */
	tCnt = 0;
	tDelayCnt = uSec * (SystemCoreClock / 1000000); 		/* ��Ҫ�Ľ����� */					  

	while(tCnt < tDelayCnt)
	{
		tCnt = DWT_CYCCNT - tStart; /* ��������У����������һ��32λ���������¼�������Ȼ������ȷ���� */		 
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
    ticks_delay = (uSec * (SystemCoreClock / (1000000))); // ��΢��������ɵδ���
    ticks_end = ticks_start + ticks_delay;

    // ticks_endû�����
    if (ticks_end >= ticks_start)
    {
        // DWT_CYCCNT��������������ʱ����û�����
        if(DWT_CYCCNT > ticks_start)
        {
            while (DWT_CYCCNT < ticks_end);
        }
        // DWT_CYCCNT���
        else
        {
            // �Ѿ���ʱ��ֱ���˳�
            return;
        }
    }
    else // ticks_end���
    {
        // DWT_CYCCNT��������������ʱ����û�����
        if(DWT_CYCCNT > ticks_start)
        {
            // �ȴ�DWT_CYCCNT��ֵ���
            while( DWT_CYCCNT > ticks_end );
        }
        // �ȴ�������DWT_CYCCNT����ticks_end
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

