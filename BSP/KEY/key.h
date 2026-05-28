/**
 ****************************************************************************************************
 * @file        key.h
 * @author      Mr.Mr.
 * @version     V1.0
 * @date        2023-04-23
 * @brief       按键驱动代码
 * @license     Copyright (c) 2020-2032,  
 ****************************************************************************************************
 * @attention
 * 
 * 实验平台:ST-1 STM32F407核心板
 *  
 *  
 * 公司网址:www.genbotter.com
 * 购买地址:makerbase.taobao.com
 * 
 ****************************************************************************************************
 */

#ifndef __KEY_H
#define __KEY_H

#include "./SYSTEM/sys/sys.h"
#include <stdbool.h>

/* 引脚定义 */
#define WKUP_GPIO_PORT          GPIOA
#define WKUP_GPIO_PIN           GPIO_PIN_0
#define WKUP_GPIO_CLK_ENABLE()  do { __HAL_RCC_GPIOA_CLK_ENABLE(); } while (0)
#define KEY0_GPIO_PORT          GPIOE
#define KEY0_GPIO_PIN           GPIO_PIN_4
#define KEY0_GPIO_CLK_ENABLE()  do { __HAL_RCC_GPIOE_CLK_ENABLE(); } while (0)
#define KEY1_GPIO_PORT          GPIOE
#define KEY1_GPIO_PIN           GPIO_PIN_3
#define KEY1_GPIO_CLK_ENABLE()  do { __HAL_RCC_GPIOE_CLK_ENABLE(); } while (0)


/* IO操作 */
#define WKUP                    ((HAL_GPIO_ReadPin(WKUP_GPIO_PORT, WKUP_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1)
#define KEY0                    ((HAL_GPIO_ReadPin(KEY0_GPIO_PORT, KEY0_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1)
#define KEY1                   ((HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_GPIO_PIN) == GPIO_PIN_RESET) ? 0 : 1)

/* 键值定义 */
#define NONE_PRES               0   /* 没有按键按下 */
#define WKUP_PRES               1   /* WKUP按键按下 */
#define KEY0_PRES               2   /* KEY0按键按下 */

/* 函数声明 */
void key_init(void);            /* 初始化按键 */
uint8_t key_scan(uint8_t mode); /* 扫描按键 */
bool key_is_pressed(uint8_t id);
#endif
