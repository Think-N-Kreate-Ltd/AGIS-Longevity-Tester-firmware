// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.1
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"

void ui_input_screen_init(void)
{
    ui_input = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_input, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_Label1 = lv_label_create(ui_input);
    lv_obj_set_width(ui_Label1, LV_SIZE_CONTENT);   /// 1
    lv_obj_set_height(ui_Label1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Label1, -87);
    lv_obj_set_y(ui_Label1, -83);
    lv_obj_set_align(ui_Label1, LV_ALIGN_CENTER);
    lv_label_set_text(ui_Label1, "Hello World!");

    ui_Dropdown1 = lv_dropdown_create(ui_input);
    lv_obj_set_width(ui_Dropdown1, 150);
    lv_obj_set_height(ui_Dropdown1, LV_SIZE_CONTENT);    /// 1
    lv_obj_set_x(ui_Dropdown1, 16);
    lv_obj_set_y(ui_Dropdown1, -37);
    lv_obj_set_align(ui_Dropdown1, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_Dropdown1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);     /// Flags

}
