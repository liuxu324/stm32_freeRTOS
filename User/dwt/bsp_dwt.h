#ifndef __DWT_H
#define	__DWT_H

#include "stm32f10x.h"

void dwt_delay_us(uint32_t us);
void dwt_delay_ms(uint16_t ms);

void dwt_delay_init(void);

#endif /* __DWT_H */

