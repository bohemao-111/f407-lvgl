#include "anim.h"

static lv_anim_t   anim_template;
static lv_anim_t * running_anim;
static uint8_t flag = 0;
static lv_obj_t * btn2;  /* 保存 btn2 引用，用于暂停/恢复 */

/* 按钮1 的点击回调 */
static void btn1_click_cb(lv_event_t *e)
{
    flag++;
    if(flag == 1)
    {
        /* 第一次点击，动画从 0→200 开始 */
        lv_anim_set_values(&anim_template, 0, 200);
        running_anim = lv_anim_start(&anim_template);
       // printf("running\n");
    }
    else if(flag == 2)
    {
        /* 第二次点击，删除动画实现暂停 */
        lv_anim_del(btn2, (lv_anim_exec_xcb_t)lv_obj_set_x);
       // printf("pause\n");
    }
    else if(flag == 3)
    {
        /* 第三次点击，从当前位置继续动画 */
        int32_t cur_x = lv_obj_get_x(btn2);
        /* ★ 修复：如果停在终点附近(≥180)，就改往起点(0)走，避免不动 */
        if(cur_x >= 180) {
            lv_anim_set_values(&anim_template, cur_x, 0);
        } else {
            lv_anim_set_values(&anim_template, cur_x, 200);
        }
        running_anim = lv_anim_start(&anim_template);
        flag = 0;
    }
}

/* 动画完成的回调 */
static void on_completed(lv_anim_t *a)
{
    (void)a;
   // printf("running is over\n");
}

void anim_create(void)
{
    lv_obj_t *scr = lv_scr_act();

    /* 按钮1：点击触发动画 */
    lv_obj_t *btn1 = lv_btn_create(scr);
    lv_obj_set_pos(btn1, 20, 100);
    lv_obj_set_size(btn1, 120, 50);
    lv_obj_t *lbl1 = lv_label_create(btn1);
    lv_label_set_text(lbl1, "click me");
    lv_obj_center(lbl1);

    /* 按钮2：被动画控制的对象 */
    btn2 = lv_btn_create(scr);
    lv_obj_set_pos(btn2, 0, 200);
    lv_obj_set_size(btn2, 120, 50);
    lv_obj_t *lbl2 = lv_label_create(btn2);
    lv_label_set_text(lbl2, "i am running");
    lv_obj_center(lbl2);

    lv_anim_init(&anim_template);

    lv_anim_set_exec_cb(&anim_template, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_var(&anim_template, btn2);
    lv_anim_set_time(&anim_template, 1000);              /* LVGL 8.3 用 set_time，不是 set_duration */
    lv_anim_set_path_cb(&anim_template, lv_anim_path_bounce);

    lv_anim_set_values(&anim_template, 0, 200);

    lv_anim_set_playback_time(&anim_template, 500);      /* 回退时长：200→0，花0.5秒 */
    lv_anim_set_playback_delay(&anim_template, 300);     /* 到终点后等 300ms 再回头 */

    lv_anim_set_ready_cb(&anim_template, on_completed);  /* 动画完成时回调（8.3：ready_cb） */

    lv_obj_add_event_cb(btn1, btn1_click_cb, LV_EVENT_CLICKED, NULL);
}
