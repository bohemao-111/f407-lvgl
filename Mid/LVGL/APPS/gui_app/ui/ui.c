#include "ui.h"

lv_obj_t *sw_led3;
lv_obj_t *sw_led2;
/* LED ???? */
static void sw_led3_cb(lv_event_t *e)
{
    if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED))
        LED3(1);
    else
        LED3(0);
}

static void sw_led2_cb(lv_event_t *e)
{
    if (lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED))
        LED2(1);
    else
        LED2(0);
}

void ui_init(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* 背景深色 */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_bg_opa(scr, 255, 0);

    /* 顶部标题栏 */
    lv_obj_t *title_bar = lv_obj_create(scr);
    lv_obj_set_size(title_bar, 320, 40);
    lv_obj_set_pos(title_bar, 0, 0);
    lv_obj_set_style_bg_color(title_bar, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(title_bar, 0, 0);
    lv_obj_set_style_radius(title_bar, 0, 0);
    lv_obj_set_style_shadow_width(title_bar, 0, 0);

    lv_obj_t *title = lv_label_create(title_bar);
    lv_label_set_text(title, "Dashboard");
    lv_obj_center(title);
    lv_obj_set_style_text_color(title, lv_color_hex(0xe94560), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);

    /* 卡片1：温度 */
    lv_obj_t *card1 = lv_obj_create(scr);
    lv_obj_set_size(card1, 145, 90);
    lv_obj_set_pos(card1, 10, 50);
    lv_obj_set_style_bg_color(card1, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(card1, 0, 0);
    lv_obj_set_style_radius(card1, 10, 0);
    lv_obj_set_style_shadow_width(card1, 10, 0);
    lv_obj_set_style_shadow_color(card1, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(card1, 80, 0);

    lv_obj_t *card1_val = lv_label_create(card1);
    lv_label_set_text(card1_val, "25.6" "\xC2\xB0");
    lv_obj_set_pos(card1_val, 10, 10);
    lv_obj_set_style_text_color(card1_val, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_text_font(card1_val, &lv_font_montserrat_14, 0);

    lv_obj_t *card1_lbl = lv_label_create(card1);
    lv_label_set_text(card1_lbl, "Temperature");
    lv_obj_set_pos(card1_lbl, 10, 60);
    lv_obj_set_style_text_color(card1_lbl, lv_color_hex(0x9b9b9b), 0);

    /* 卡片2：湿度 */
    lv_obj_t *card2 = lv_obj_create(scr);
    lv_obj_set_size(card2, 145, 90);
    lv_obj_set_pos(card2, 165, 50);
    lv_obj_set_style_bg_color(card2, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(card2, 0, 0);
    lv_obj_set_style_radius(card2, 10, 0);
    lv_obj_set_style_shadow_width(card2, 10, 0);
    lv_obj_set_style_shadow_color(card2, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(card2, 80, 0);

    lv_obj_t *card2_val = lv_label_create(card2);
    lv_label_set_text(card2_val, "65%");
    lv_obj_set_pos(card2_val, 10, 10);
    lv_obj_set_style_text_color(card2_val, lv_color_hex(0x2195f6), 0);
    lv_obj_set_style_text_font(card2_val, &lv_font_montserrat_14, 0);

    lv_obj_t *card2_lbl = lv_label_create(card2);
    lv_label_set_text(card2_lbl, "Humidity");
    lv_obj_set_pos(card2_lbl, 10, 60);
    lv_obj_set_style_text_color(card2_lbl, lv_color_hex(0x9b9b9b), 0);

    /* 卡片3：控制面板 */
    lv_obj_t *card3 = lv_obj_create(scr);
    lv_obj_set_size(card3, 300, 85);
    lv_obj_set_pos(card3, 10, 150);
    lv_obj_set_style_bg_color(card3, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_border_width(card3, 0, 0);
    lv_obj_set_style_radius(card3, 10, 0);
    lv_obj_set_style_shadow_width(card3, 10, 0);
    lv_obj_set_style_shadow_color(card3, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(card3, 80, 0);

    lv_obj_t *ctrl_lbl = lv_label_create(card3);
    lv_label_set_text(ctrl_lbl, "Controls");
    lv_obj_set_pos(ctrl_lbl, 15, 10);
    lv_obj_set_style_text_color(ctrl_lbl, lv_color_hex(0xffffff), 0);

    /* LED3 开关 */
    sw_led3 = lv_switch_create(card3);
    lv_obj_set_pos(sw_led3, 15, 40);
    lv_obj_set_style_bg_color(sw_led3, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw_led3, lv_color_hex(0x2195f6), LV_PART_INDICATOR);

    lv_obj_t *sw1_lbl = lv_label_create(card3);
    lv_label_set_text(sw1_lbl, "LED3");
    lv_obj_set_pos(sw1_lbl, 60, 45);
    lv_obj_set_style_text_color(sw1_lbl, lv_color_hex(0x9b9b9b), 0);

    /* LED2 开关 */
    sw_led2 = lv_switch_create(card3);
    lv_obj_set_pos(sw_led2, 120, 40);
    lv_obj_set_style_bg_color(sw_led2, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(sw_led2, lv_color_hex(0xe94560), LV_PART_INDICATOR);

    lv_obj_t *sw2_lbl = lv_label_create(card3);
    lv_label_set_text(sw2_lbl, "LED2");
    lv_obj_set_pos(sw2_lbl, 165, 45);
    lv_obj_set_style_text_color(sw2_lbl, lv_color_hex(0x9b9b9b), 0);

    /* 亮度滑块 */
    lv_obj_t *slider_lbl = lv_label_create(card3);
    lv_label_set_text(slider_lbl, "Bright");
    lv_obj_set_pos(slider_lbl, 200, 10);
    lv_obj_set_style_text_color(slider_lbl, lv_color_hex(0x9b9b9b), 0);

    lv_obj_t *slider = lv_slider_create(card3);
    lv_obj_set_pos(slider, 200, 35);
    lv_obj_set_size(slider, 85, 15);
    lv_slider_set_value(slider, 70, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xe94560), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xffffff), LV_PART_KNOB);




    lv_obj_add_event_cb(sw_led3, sw_led3_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_add_event_cb(sw_led2, sw_led2_cb, LV_EVENT_VALUE_CHANGED, NULL);
}
