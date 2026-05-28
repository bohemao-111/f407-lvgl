#ifndef __DELAY_H
#define __DELAY_H

#include "./SYSTEM/sys/sys.h"

void delay_init(uint32_t sysclk);
void delay_us(uint32_t nus);
void delay_ms(uint16_t nms);

#endif
