#ifndef __SYS_H
#define __SYS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* 位操作 */
#define BIT(n)                  (1U << (n))
#define BIT_SET(reg, bit)       ((reg) |= BIT(bit))
#define BIT_CLR(reg, bit)       ((reg) &= ~BIT(bit))
#define BIT_GET(reg, bit)       (((reg) >> (bit)) & 1)

/* 中断开关 */
#define SYS_INT_ENABLE()        __enable_irq()
#define SYS_INT_DISABLE()       __disable_irq()

void sys_clock_enable(void);

#endif
