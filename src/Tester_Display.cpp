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
  lv_style_set_pad_all(&style, 1);

  /*set the info % ins widget(container)*/
  set_infoarea(screenMain);
  set_insarea(screenMain);

  /*set the pattern container*/
  lv_obj_t * pat1_cont = lv_obj_create(screenMain);
  set_patarea(pat1_cont, 1);
  lv_obj_t * pat2_cont = lv_obj_create(screenMain);
  set_patarea(pat2_cont, 2);

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

// setting text in grid, with specific grid and no padding
void set_grid_obj(lv_obj_t * parent, uint8_t col_pos, uint8_t col_span, uint8_t row_pos, uint8_t row_span, const char * text) {
  lv_obj_t * obj = lv_obj_create(parent);
  lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col_pos, col_span, LV_GRID_ALIGN_CENTER, row_pos, row_span);
  lv_obj_t * label = lv_label_create(obj);
  lv_label_set_text(label, text);
  lv_obj_set_style_pad_all(obj, 0, 0);
}

// setting input field in grid, with specific grid and no padding
void set_grid_obj_input(lv_obj_t * parent, uint8_t col_pos, uint8_t col_span, uint8_t row_pos, uint8_t row_span, const char * text, uint8_t index) {
  lv_obj_t * obj = lv_textarea_create(parent);
  lv_obj_set_size(obj, 30, LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col_pos, col_span, LV_GRID_ALIGN_CENTER, row_pos, row_span);
  
  lv_textarea_set_one_line(obj, true);
  lv_textarea_set_max_length(obj, 3);
  lv_textarea_set_placeholder_text(obj, text);

  lv_obj_move_to_index(obj, index);
  lv_obj_add_event_cb(obj, textarea_event_cb, LV_EVENT_ALL, obj);

  lv_obj_set_style_pad_all(obj, 0, 0);
}

void set_patarea(lv_obj_t * obj, uint8_t index) {
  static lv_coord_t col_dsc[] = {lv_pct(13), lv_pct(15), lv_pct(13), lv_pct(15), lv_pct(21), lv_pct(21), LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {lv_pct(11), lv_pct(12), lv_pct(12), LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(obj, col_dsc, row_dsc);
  lv_obj_set_size(obj, lv_pct(100), lv_pct(36));

  /*set label*/
  set_grid_obj(obj, 1, 1, 0, 1, "Timeout\n(s)");
  set_grid_obj(obj, 2, 1, 0, 1, "PWM");
  set_grid_obj(obj, 3, 1, 0, 1, "Number\nof times");
  set_grid_obj(obj, 4, 2, 0, 1, "Stop condition");

  set_grid_obj(obj, 0, 1, 1, 1, "T_up_\npattern");
  set_grid_obj(obj, 4, 2, 1, 1, "Upper LS touched");

  set_grid_obj(obj, 0, 1, 2, 1, "T_up_\npattern");
  set_grid_obj(obj, 4, 2, 2, 1, "Upper LS touched");

  /*set input field*/
  set_grid_obj_input(obj, 1, 1, 1, 1, "10", T_OUT_UP_INDEX);
  set_grid_obj_input(obj, 1, 1, 2, 1, "10", T_OUT_DOWN_INDEX);
  set_grid_obj_input(obj, 2, 1, 1, 1, "100", PWM_UP_INDEX);
  set_grid_obj_input(obj, 2, 1, 2, 1, "60", PWM_DOWN_INDEX);
  set_grid_obj_input(obj, 3, 1, 1, 2, "2", NUM_TIME_INDEX);

  /*setting with different index*/
  if (index == 1) {
    set_grid_obj(obj, 0, 1, 0, 1, "Action\npattern1");
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);
  } else if (index == 2) {
    set_grid_obj(obj, 0, 1, 0, 1, "Action\npattern2");
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, lv_pct(28));
  }
  lv_obj_move_to_index(obj, index);

  /*style*/
  lv_obj_set_style_pad_all(obj, 0, 0);
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
}

static void textarea_event_cb(lv_event_t * event) {  
  lv_event_code_t code = lv_event_get_code(event);
  lv_obj_t * ta = lv_event_get_target(event);

  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    /*when text area is clicked*/
    /*do nothing*/
  } else if (code == LV_EVENT_READY) {
    /*get the input and store it*/
    uint16_t i = lv_obj_get_index(ta);
    TT = atoi(lv_textarea_get_text(ta));
  }
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