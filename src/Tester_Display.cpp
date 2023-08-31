#include <Tester_Display.h>
#include <Keypad.h>

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[TFT_WIDTH * TFT_HEIGHT / 10];

TFT_eSPI tft = TFT_eSPI();

lv_group_t * grp;           /*a group to group all keypad evented object*/
lv_obj_t * screenMain;      /*a screen object which will hold all other objects for input*/
lv_obj_t * screenMonitor;   /*a screen object which will hold all other objects for data display*/
lv_indev_t * keypad_indev;  /*a driver in LVGL and save the created input device object*/
static lv_style_t style;    /*set the layout style*/

void display_init() {
  tft.begin();
  tft.setRotation(3);        // Landscape orientation

  lv_init();
  lv_disp_draw_buf_init(&disp_buf, buf, NULL, TFT_WIDTH * TFT_HEIGHT / 10);

  /*Initialize the display driver*/
  static lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_HEIGHT;  /*flipped since we use horizontal view*/
  disp_drv.ver_res = TFT_WIDTH;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.draw_buf = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  /*Initialize the input device driver, e.g. keypad, touch screen*/
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_KEYPAD;
  indev_drv.read_cb = keypad_read;  // function of reading data from keypad
  /*Register the driver in LVGL and save the created input device object*/
  keypad_indev = lv_indev_drv_register(&indev_drv);
  /*set group*/
  grp = lv_group_create();
  lv_group_set_default(grp);  /*let the object create added to this group*/
  lv_indev_set_group(keypad_indev, grp);

  // Call every 500ms // TODO:
  // lv_timer_t * infusion_monitoring_timer = lv_timer_create(infusion_monitoring_cb, 500, NULL);
}

/*writes color information from the “color_p” pointer to the needed “area”*/
void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushColors(( uint16_t * )&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp_drv);
}

void input_screen() {
  /*a screen object which will hold all other objects*/
  screenMain = lv_obj_create(NULL);

  /*set the layout style*/
  lv_style_set_layout(&style, LV_LAYOUT_FLEX);
  lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
  lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_AROUND);
  lv_style_set_flex_cross_place(&style, LV_FLEX_ALIGN_CENTER);
  lv_style_set_pad_all(&style, 2);

  set_infoarea(screenMain);
  set_insarea(screenMain);

  /*Loads the main screen*/
  lv_disp_load_scr(screenMain);
}

void set_infoarea(lv_obj_t * parent) {
  /*create the object for the first container*/
  lv_obj_t * widget = lv_obj_create(parent);
  lv_obj_set_style_border_color(widget, lv_color_hex(0x5b5b5b), LV_PART_MAIN);
  lv_obj_set_style_radius(widget, 0x00, LV_PART_MAIN);
  lv_obj_set_size(widget, lv_pct(60), lv_pct(23));
  lv_obj_align(widget, LV_ALIGN_TOP_LEFT, 3, 3);
  lv_obj_set_scrollbar_mode(widget, LV_SCROLLBAR_MODE_OFF);

  /*create objects in container*/
  lv_obj_t * id_label = lv_label_create(widget);
  lv_label_set_text(id_label, "Sample ID:");
  lv_obj_t * id2_label = lv_label_create(widget);
  lv_label_set_text(id2_label, "Numerical input only:");
  lv_obj_t * date_label = lv_label_create(widget);
  lv_label_set_text(date_label, "Date & Time:");
  lv_obj_t * date2_label = lv_label_create(widget);
  lv_label_set_text(date2_label, "dateTime0123");
  lv_obj_t * load_label = lv_label_create(widget);
  lv_label_set_text(load_label, "Load Profile:");
  lv_obj_t * load2_label = lv_label_create(widget);
  lv_label_set_text(load2_label, "Default");

  lv_obj_add_style(widget, &style, 0);
}

void set_insarea(lv_obj_t * parent) {
  /*create the object for the first container*/
  lv_obj_t * widget = lv_obj_create(parent);
  lv_obj_set_style_border_color(widget, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_radius(widget, 0x00, LV_PART_MAIN);
  lv_obj_set_size(widget, lv_pct(30), lv_pct(24));
  lv_obj_align(widget, LV_ALIGN_TOP_RIGHT, -7, 7);
  lv_obj_set_scrollbar_mode(widget, LV_SCROLLBAR_MODE_OFF);

  /*create objects in container*/
  lv_obj_t * ins_label = lv_label_create(widget);
  lv_label_set_text(ins_label, "INSTRUCTIONS");
  lv_obj_t * key1_label = lv_label_create(widget);
  lv_label_set_text(key1_label, "F1: Clear all");
  lv_obj_t * key2_label = lv_label_create(widget);
  lv_label_set_text(key2_label, "F2: Run");

  lv_obj_add_style(widget, &style, 0);
}

void keypad_read(lv_indev_drv_t * drv, lv_indev_data_t * data){
  uint8_t key = keypad.getKey();
  if(key) {
    Serial.write(key);
    if (key == 'E') {
      data->key = LV_KEY_ENTER;
    }
    else if (key == 'C') {
      data->key = LV_KEY_ESC;
    }
    else if (key == 'L') {
      data->key = LV_KEY_LEFT;
      // data->key = LV_KEY_PREV;
    }
    else if (key == 'R') {
      data->key = LV_KEY_RIGHT;
      // data->key = LV_KEY_NEXT;
    }
    else if (key == 'U') {
      data->key = LV_KEY_UP;
    }
    else if (key == 'D') {
      data->key = LV_KEY_DOWN;
    }
    else if (key == '#') {
      data->key = LV_KEY_BACKSPACE;
    }
    else if (key == '*') {
    }
    else if (key == 'F') {
    }
    else if (key == 'G') {
    }
    else {
      data->key = key;  /*must enter here, for input numbers*/
    }

    data->state = LV_INDEV_STATE_PRESSED;
  }
  else if (keypad.getState() == 0) {  // when keypad pressing is released
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = 0x00; /*reset key value and do nothing*/
  }
}