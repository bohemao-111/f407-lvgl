#ifndef __UI_H__
#define __UI_H__

#include "lvgl.h"
#include "./BSP/LED/led.h"

/* 开关控件，供 main.c 注册事件回调 */
extern lv_obj_t *sw_led3;
extern lv_obj_t *sw_led2;

void ui_init(void);

#endif // __UI_H__
