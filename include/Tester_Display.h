#ifndef B06D06F7_A331_4A0C_9C45_FE06A1E5D553
#define B06D06F7_A331_4A0C_9C45_FE06A1E5D553

#include <TFT_eSPI.h>
#include <lvgl.h>
#include <lv_conf.h>
#include <Tester_Keypad.h>
#include <Tester_common.h>

#define T_OUT_UP_INDEX    0
#define T_OUT_DOWN_INDEX  1
#define PWM_UP_INDEX      0
#define PWM_DOWN_INDEX    1
#define NUM_TIME_INDEX    2

void display_init();
void input_screen();
void monitor_screen();

/*----------function prototype only use in AGIS_Display.h (private)----------*/

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
void set_infoarea(lv_obj_t * parent);
void set_insarea(lv_obj_t * parent);
void set_grid_obj(lv_obj_t * parent, uint8_t col_pos, uint8_t col_span, uint8_t row_pos, uint8_t row_span, const char * text);
void set_patarea(lv_obj_t * obj, uint8_t index);

void keypad_read(lv_indev_drv_t * drv, lv_indev_data_t * data);
void infusion_monitoring_cb(lv_timer_t * timer);

#endif /* B06D06F7_A331_4A0C_9C45_FE06A1E5D553 */
