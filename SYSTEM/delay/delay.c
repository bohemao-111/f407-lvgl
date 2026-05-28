#include "./SYSTEM/delay/delay.h"

static uint32_t g_delay_fac_us = 0;  /* 1微秒需要的DWT计数值 */

/**
 * @brief   延时初始化 (使用DWT内核计数器)
 * @param   sysclk: 系统时钟频率(Hz)
 */
void delay_init(uint32_t sysclk)
{
    g_delay_fac_us = sysclk / 1000000;  /* 1us的计数值 */

    /* 使能DWT外设 */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
 * @brief   微秒延时(使用DWT, 短延时精准)
 * @param   nus: 延时微秒数
 */
void delay_us(uint32_t nus)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = nus * g_delay_fac_us;

    while ((DWT->CYCCNT - start) < ticks);
}

/**
 * @brief   毫秒延时
 *   @note  使用HAL_Delay实现, 可在中断中调用
 * @param   nms: 延时毫秒数
 */
void delay_ms(uint16_t nms)
{
    HAL_Delay(nms);
}
