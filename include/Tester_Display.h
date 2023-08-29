#ifndef B06D06F7_A331_4A0C_9C45_FE06A1E5D553
#define B06D06F7_A331_4A0C_9C45_FE06A1E5D553

#include <TFT_eSPI.h>
#include <lvgl.h>
#include <lv_conf.h>
#include <Tester_Keypad.h>

void display_init();

/*----------function prototype only use in AGIS_Display.h (private)----------*/

void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p );
void keypad_read(lv_indev_drv_t * drv, lv_indev_data_t * data);

#endif /* B06D06F7_A331_4A0C_9C45_FE06A1E5D553 */
