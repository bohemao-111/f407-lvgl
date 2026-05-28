#include "./SYSTEM/usart/usart.h"
#include <stdio.h>

/* CubeMX生成的USART句柄, 声明在Core/Inc/usart.h */
/* 注意: 不能写#include "usart.h", 因为会和本目录下的usart.h冲突 */
extern UART_HandleTypeDef huart1;

/* Keil MDK printf重定向 (使用MicroLib时自动生效) */
#if defined(__MICROLIB)
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
#else
/* 标准C库printf重定向 */
#pragma import(__use_no_semihosting)

struct __FILE { int handle; };
FILE __stdout;

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}

void _sys_exit(int x)
{
    (void)x;
}

void _ttywrch(int ch)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFF);
}
#endif

/**
 * @brief   初始化USART(printf重定向)
 * @param   baudrate: 波特率(本项目由CubeMX初始化, 此参数保留兼容性)
 */
void usart_init(uint32_t baudrate)
{
    (void)baudrate;
    /* CubeMX已初始化USART1, 此处无需额外操作 */
}
