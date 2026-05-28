/*
* Copyright 2026 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

typedef struct
{
  
	lv_obj_t *screen;
	bool screen_del;
	lv_obj_t *screen_btn_led_3;
	lv_obj_t *screen_btn_led_3_label;
	lv_obj_t *screen_btn_led_2;
	lv_obj_t *screen_btn_led_2_label;
	lv_obj_t *screen_label_1;
	lv_obj_t *screen_ta_1;
	lv_obj_t *screen_img_1;
	lv_obj_t *screen_circle_1;
}lv_ui;

void ui_init_style(lv_style_t * style);
void init_scr_del_flag(lv_ui *ui);
void setup_ui(lv_ui *ui);
extern lv_ui guider_ui;

void setup_scr_screen(lv_ui *ui);
LV_IMG_DECLARE(_b_400a6bc1f6346112b83920ce7b788aa9_alpha_84x79);

LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_montserratMedium_12)
LV_FONT_DECLARE(lv_font_ArchitectsDaughter_16)
LV_FONT_DECLARE(lv_font_simsun_14)


#ifdef __cplusplus
}
#endif
#endif
