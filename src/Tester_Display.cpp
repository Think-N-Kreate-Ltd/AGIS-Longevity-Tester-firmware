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
  tft.setRotation(1);        // Landscape orientation

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
  lv_indev_set_group(keypad_indev, grp);

  // Call every 500ms
  lv_timer_t * infusion_monitoring_timer = lv_timer_create(infusion_monitoring_cb, 500, NULL);
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

  /*set the info & ins widget(container)*/
  set_infoarea(true);
  set_insarea(true);

  /*set the pattern container*/
  lv_obj_t * pat1_cont = lv_obj_create(screenMain);
  set_patarea(pat1_cont, 1);
  lv_obj_t * pat2_cont = lv_obj_create(screenMain);
  set_patarea(pat2_cont, 2);

  /*Loads the main screen*/
  lv_disp_load_scr(screenMain);
}

void monitor_screen() {
  /*a screen object which will hold all other objects*/
  screenMonitor = lv_obj_create(NULL);

  /*set the info & ins widget(container)*/
  set_infoarea(false);
  set_insarea(false);

  /*set the table for showing status*/
  lv_obj_t * table = lv_table_create(screenMonitor);
  lv_obj_move_to_index(table, 1);

  lv_obj_align(table, LV_ALIGN_BOTTOM_MID, 0, 0);
  // lv_obj_set_style_border_color(table, lv_color_hex(0x5b5b5b), LV_PART_MAIN);
  lv_obj_set_style_border_opa(table, LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_line_color(table, lv_color_hex(0x5b5b5b), LV_PART_MAIN);
  lv_table_set_col_width(table, 0, 140);
  lv_obj_set_size(table, lv_pct(99), lv_pct(70));
  lv_obj_set_style_text_font(table, &lv_font_montserrat_14, 0);

  /*Fill the first column*/
  lv_table_set_cell_value(table, 0, 0, "No. of cycles:");
  lv_table_set_cell_value(table, 1, 0, "Status (current action)");
  lv_table_set_cell_value(table, 2, 0, "Motor run time\n(HHH:MM:SS)");
  lv_table_set_cell_value(table, 3, 0, "Tester state");  // TODO: think about this text

  /*Fill the second column*/
  for (int i=0; i<4; ++i) {
    lv_table_set_cell_value(table, i, 1, "N/A");
  }

  /*Loads the monitor screen if resume*/
  if (resumeAfterCutOff) {
    lv_disp_load_scr(screenMonitor);
  }
}


// screen=true for input screen
// screen=false for monitor screen
void set_infoarea(bool screen) {
  /*create the object for the first container*/
  lv_obj_t * widget;
  if (screen) {
    widget = lv_obj_create(screenMain);
  } else {
    widget = lv_obj_create(screenMonitor);
  }
  lv_obj_set_style_border_color(widget, lv_color_hex(0x5b5b5b), LV_PART_MAIN);
  lv_obj_set_style_radius(widget, 0x00, LV_PART_MAIN);
  lv_obj_set_size(widget, lv_pct(60), lv_pct(24));
  lv_obj_align(widget, LV_ALIGN_TOP_LEFT, 3, 3);
  lv_obj_set_scrollbar_mode(widget, LV_SCROLLBAR_MODE_OFF);

  /*create objects in container*/
  lv_obj_t * id_label = lv_label_create(widget);
  lv_label_set_text(id_label, "Sample ID:");
  if (screen) {
    lv_obj_t * id_input = lv_textarea_create(widget);
    lv_textarea_set_one_line(id_input, true);
    lv_textarea_set_max_length(id_input, 8);
    lv_obj_set_width(id_input, 60);
    char default_data[12];
    sprintf(default_data, "%d", sampleId);
    lv_textarea_set_placeholder_text(id_input, default_data);
    lv_obj_add_event_cb(id_input, id_input_event_cb, LV_EVENT_ALL, id_input);
    lv_obj_set_style_pad_all(id_input, 0, 0);
    lv_group_add_obj(grp, id_input);
    lv_obj_move_to_index(id_input, 1);  /*in fact, it is 1 originally*/
  } else {
    lv_obj_t * id_input_label = lv_label_create(widget);
    lv_label_set_text(id_input_label, "Now loading");
    lv_obj_move_to_index(id_input_label, 1);
    lv_obj_set_width(id_input_label, 70);
  }

  lv_obj_t * date_label = lv_label_create(widget);
  lv_label_set_text(date_label, "Date & Time:");
  lv_obj_t * dateTime_obj = lv_label_create(widget);
  lv_label_set_text(dateTime_obj, "Have no WiFi yet");
  lv_obj_move_to_index(dateTime_obj, 3);  /*in fact, it is 3 originally*/

  lv_obj_t * load_label = lv_label_create(widget);
  lv_label_set_text(load_label, "Load Profile:");
  lv_obj_t * load2_label = lv_label_create(widget);
  lv_label_set_text(load2_label, "Default");
  lv_obj_set_width(load2_label, 60);

  lv_obj_add_style(widget, &style, 0);
  lv_obj_move_to_index(widget, 0);
}

// screen=true for input screen
// screen=false for monitor screen
void set_insarea(bool screen) {
  /*create the object for the first container*/
  lv_obj_t * widget;
  if (screen) {
    widget = lv_obj_create(screenMain);
  } else {
    widget = lv_obj_create(screenMonitor);
  }
  lv_obj_set_style_border_opa(widget, LV_OPA_0, 0);
  lv_obj_set_size(widget, lv_pct(30), lv_pct(24));
  lv_obj_align(widget, LV_ALIGN_TOP_RIGHT, -7, 7);
  lv_obj_set_scrollbar_mode(widget, LV_SCROLLBAR_MODE_OFF);

  /*create objects in container*/
  lv_obj_t * ins_label = lv_label_create(widget);
  lv_label_set_text(ins_label, "INSTRUCTIONS");
  lv_obj_set_style_text_decor(ins_label, LV_TEXT_DECOR_UNDERLINE, 0);

  lv_obj_t * key1_label = lv_label_create(widget);
  lv_obj_t * key2_label = lv_label_create(widget);

  if (screen) {
    lv_label_set_text(key1_label, "ESC: Clear all");
    lv_label_set_text(key2_label, "F2: Run");
  } else {
    lv_label_set_text(key1_label, "*:Pause/Resume");
    lv_label_set_text(key2_label, "**:Stop");
  }
  lv_obj_set_style_bg_color(key1_label, lv_color_hex(0x00EB00), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(key1_label, LV_OPA_50, LV_PART_MAIN);
  lv_obj_set_style_bg_color(key2_label, lv_color_hex(0xFF0000), LV_PART_MAIN);
  lv_obj_set_style_bg_opa(key2_label, LV_OPA_50, LV_PART_MAIN);

  lv_obj_add_style(widget, &style, 0);
}

// setting text in grid, with specific grid and no padding
void set_grid_obj(lv_obj_t * parent, uint8_t col_pos, uint8_t col_span, uint8_t row_pos, uint8_t row_span, const char * text) {
  lv_obj_t * obj = lv_obj_create(parent);
  lv_obj_set_size(obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col_pos, col_span, LV_GRID_ALIGN_CENTER, row_pos, row_span);
  lv_obj_t * label = lv_label_create(obj);
  lv_label_set_text(label, text);
  
  lv_obj_set_style_border_opa(obj, LV_OPA_0, 0);
  if (row_pos == 0) {
    /*set the first row bg color*/
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xBFBFBF), LV_PART_MAIN);
    lv_obj_set_height(obj, lv_pct(33));
    lv_obj_set_style_pad_ver(obj, 0, 0);
    lv_obj_set_style_radius(obj, 0x00, LV_PART_MAIN);
  } else {
    lv_obj_set_style_pad_all(obj, 0, 0);
  }
}

// setting input field in grid, with specific grid and no padding
// pattern = pattern number
void set_grid_obj_input(lv_obj_t * parent, uint8_t col_pos, uint8_t col_span, uint8_t row_pos, uint8_t row_span, const char * text, uint8_t index, uint8_t pattern) {
  lv_obj_t * obj = lv_textarea_create(parent);
  lv_obj_set_size(obj, 30, LV_SIZE_CONTENT);
  lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_CENTER, col_pos, col_span, LV_GRID_ALIGN_CENTER, row_pos, row_span);
  
  lv_textarea_set_one_line(obj, true);
  lv_textarea_set_max_length(obj, 3);
  lv_textarea_set_text(obj, text);

  lv_obj_move_to_index(obj, index);
  if (pattern == 1) {
    lv_obj_add_event_cb(obj, pat1_event_cb, LV_EVENT_ALL, obj);
  } else if (pattern == 2) {
    lv_obj_add_event_cb(obj, pat2_event_cb, LV_EVENT_ALL, obj);
  }
  
  lv_group_add_obj(grp, obj);

  lv_obj_set_style_pad_all(obj, 0, 0);
  lv_obj_set_style_border_opa(obj, LV_OPA_0, 0);
  lv_obj_set_style_text_align(obj, LV_TEXT_ALIGN_CENTER, 0);  // note: lv_textarea_set_align is decrecated
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

  set_grid_obj(obj, 0, 1, 2, 1, "T_down_\npattern");
  set_grid_obj(obj, 4, 2, 2, 1, "Lower LS touched");

  /*setting with different index*/
  char default_data[4];
  if (index == 1) {
    set_grid_obj(obj, 0, 1, 0, 1, "Action\npat1");
    set_grid_obj(obj, 4, 2, 1, 1, "Upper LS touched");
    sprintf(default_data, "%d", T_OUT_P1UP);
    set_grid_obj_input(obj, 1, 1, 1, 1, default_data, T_OUT_UP_INDEX, index);
    sprintf(default_data, "%d", T_OUT_P1DOWN);
    set_grid_obj_input(obj, 1, 1, 2, 1, default_data, T_OUT_DOWN_INDEX, index);
    sprintf(default_data, "%d", PWM_P1UP*100/255);
    set_grid_obj_input(obj, 2, 1, 1, 1, default_data, PWM_UP_INDEX, index);
    sprintf(default_data, "%d", abs(PWM_P1DOWN*100/255));
    set_grid_obj_input(obj, 2, 1, 2, 1, default_data, PWM_DOWN_INDEX, index);
    sprintf(default_data, "%d", (numTime_P1+1)/2);
    set_grid_obj_input(obj, 3, 1, 1, 2, default_data, NUM_TIME_INDEX, index);
    lv_obj_align(obj, LV_ALIGN_TOP_MID, 0, lv_pct(28));
  } else if (index == 2) {
    set_grid_obj(obj, 0, 1, 0, 1, "Action\npat2");
    sprintf(default_data, "%d", T_OUT_P2UP);
    set_grid_obj_input(obj, 1, 1, 1, 1, default_data, T_OUT_UP_INDEX, index);
    sprintf(default_data, "%d", T_OUT_P2DOWN);
    set_grid_obj_input(obj, 1, 1, 2, 1, default_data, T_OUT_DOWN_INDEX, index);
    sprintf(default_data, "%d", PWM_P2UP*100/255);
    set_grid_obj_input(obj, 2, 1, 1, 1, default_data, PWM_UP_INDEX, index);
    sprintf(default_data, "%d", abs(PWM_P2DOWN*100/255));
    set_grid_obj_input(obj, 2, 1, 2, 1, default_data, PWM_DOWN_INDEX, index);
    sprintf(default_data, "%d", (numTime_P2+1)/2);
    set_grid_obj_input(obj, 3, 1, 1, 2, default_data, NUM_TIME_INDEX, index);
    sprintf(default_data, "%ds", T_P2running);
    set_grid_obj_input(obj, 4, 2, 1, 1, default_data, T_RUN_UP_INDEX, index);
    lv_obj_align(obj, LV_ALIGN_BOTTOM_MID, 0, 0);
  }
  lv_obj_move_to_index(obj, index);

  /*style*/
  lv_obj_set_style_pad_all(obj, 0, 0);
  lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
  lv_obj_set_style_border_opa(obj, LV_OPA_0, 0);
}

static void pat1_event_cb(lv_event_t * event) {  
  lv_event_code_t code = lv_event_get_code(event);
  lv_obj_t * ta = lv_event_get_target(event);

  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    /*when text area is clicked*/
    /*do nothing*/
  } else if (code == LV_EVENT_READY) {
    /*get the input and store it*/
    uint16_t i = lv_obj_get_index(ta);
    Serial.println(i);
    if (i == T_OUT_UP_INDEX) {
      T_OUT_P1UP = atoi(lv_textarea_get_text(ta));
    } else if (i == T_OUT_DOWN_INDEX) {
      T_OUT_P1DOWN = atoi(lv_textarea_get_text(ta));
    } else if (i == PWM_UP_INDEX) {
      PWM_P1UP = atoi(lv_textarea_get_text(ta)) * 255 / 100;
    } else if (i == PWM_DOWN_INDEX) {
      PWM_P1DOWN = atoi(lv_textarea_get_text(ta)) * 255 / 100;
      PWM_P1DOWN *= -1;
    } else if (i == NUM_TIME_INDEX) {
      numTime_P1 = atoi(lv_textarea_get_text(ta)) * 2 - 1;
    }
    /*change the input color*/
    lv_obj_set_style_text_color(lv_obj_get_child(lv_obj_get_child(screenMain, 1), i), lv_palette_main(LV_PALETTE_GREEN), 0);
  }
}

static void pat2_event_cb(lv_event_t * event) {  
  lv_event_code_t code = lv_event_get_code(event);
  lv_obj_t * ta = lv_event_get_target(event);

  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    /*when text area is clicked*/
    /*do nothing*/
  } else if (code == LV_EVENT_READY) {
    /*get the input and store it*/
    uint16_t i = lv_obj_get_index(ta);
    if (i == T_OUT_UP_INDEX) {
      T_OUT_P2UP = atoi(lv_textarea_get_text(ta));
    } else if (i == T_OUT_DOWN_INDEX) {
      T_OUT_P2DOWN = atoi(lv_textarea_get_text(ta));
    } else if (i == PWM_UP_INDEX) {
      PWM_P2UP = atoi(lv_textarea_get_text(ta)) * 255 / 100;
    } else if (i == PWM_DOWN_INDEX) {
      PWM_P2DOWN = atoi(lv_textarea_get_text(ta)) * 255 / 100;
      PWM_P2DOWN *= -1;
    } else if (i == NUM_TIME_INDEX) {
      numTime_P2 = atoi(lv_textarea_get_text(ta)) * 2 - 1;
    } else if (i == T_RUN_UP_INDEX) {
      T_P2running = atoi(lv_textarea_get_text(ta));
    }
    /*change the input color*/
    lv_obj_set_style_text_color(lv_obj_get_child(lv_obj_get_child(screenMain, 2), i), lv_palette_main(LV_PALETTE_GREEN), 0);
  }
}

static void id_input_event_cb(lv_event_t * event) {  
  lv_event_code_t code = lv_event_get_code(event);
  lv_obj_t * ta = lv_event_get_target(event);

  if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
    /*when text area is clicked*/
    /*do nothing*/
  } else if (code == LV_EVENT_READY) {
    /*get the input and store it*/
    sampleId = atoi(lv_textarea_get_text(ta));
    /*change the input color*/
    lv_obj_set_style_text_color(lv_obj_get_child(lv_obj_get_child(screenMain, 0), 1), lv_palette_main(LV_PALETTE_GREEN), 0);
  }
}

void infusion_monitoring_cb(lv_timer_t * timer) {
  if (strchr(dateTime, ':')) {  // keep show last text when not connect to WiFi
    lv_label_set_text(lv_obj_get_child(lv_obj_get_child(screenMain, 0), 3), dateTime);
    lv_label_set_text(lv_obj_get_child(lv_obj_get_child(screenMonitor, 0), 3), dateTime);
  }

  if (status.testState && (status.cycleState!=0)) {  // when start the test, update the monitor screen
    static bool doOnceOnly = true;  // if need to run test more than one time, this statement should move outside
    if (doOnceOnly) { // only do once for update info
      lv_label_set_text_fmt(lv_obj_get_child(lv_obj_get_child(screenMonitor, 0), 1), "%08d", sampleId);
      doOnceOnly = false;
    }

    /*update table when testing*/
    lv_table_set_cell_value_fmt(lv_obj_get_child(screenMonitor, 1), 0, 1, "%d", status.numCycle);
    if (status.motorState) {
      lv_table_set_cell_value_fmt(lv_obj_get_child(screenMonitor, 1), 1, 1, "Pattern %d > Motor Up", status.cycleState);
    } else {
      lv_table_set_cell_value_fmt(lv_obj_get_child(screenMonitor, 1), 1, 1, "Pattern %d > Motor Down", status.cycleState);
    }
    uint16_t hour = status.motorRunTime/3600;
    uint8_t min = status.motorRunTime%3600/60;
    uint8_t sec = status.motorRunTime%60;
    lv_table_set_cell_value_fmt(lv_obj_get_child(screenMonitor, 1), 2, 1, "%03d:%02d:%02d", hour, min, sec);
    if (status.pauseState) {
      lv_table_set_cell_value(lv_obj_get_child(screenMonitor, 1), 3, 1, "Paused");
    } else if (failReason == failReason_t::NOT_YET) {
      lv_table_set_cell_value(lv_obj_get_child(screenMonitor, 1), 3, 1, "N/A");
    }
  }

  /*update failure reason*/
  static bool doOnce = true;  // if need to run test more than one time, this statement should move outside
  if (failReason != failReason_t::NOT_YET && doOnce) {
    if (failReason == failReason_t::CURRENT_EXCEED) {
      lv_table_set_cell_value(lv_obj_get_child(screenMonitor, 1), 3, 1, "Touch stall current");
    }
    if (failReason == failReason_t::TIME_OUT) {
      lv_table_set_cell_value(lv_obj_get_child(screenMonitor, 1), 3, 1, "Time out");
    }
    if (failReason == failReason_t::PRESS_KEY) {
      lv_table_set_cell_value(lv_obj_get_child(screenMonitor, 1), 3, 1, "Key `*` pressed");
    }
    doOnce = false;
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
      esp_restart();
    }
    else if (key == 'L') {
      data->key = LV_KEY_LEFT;
    }
    else if (key == 'R') {
      data->key = LV_KEY_RIGHT;
    }
    else if (key == 'U') {
      data->key = LV_KEY_PREV;
      if (failReason != failReason_t::NOT_YET || status.pauseState) {
        lv_obj_scroll_by(lv_obj_get_child(screenMonitor, 1), 0, 50, LV_ANIM_ON);
      }
    }
    else if (key == 'D') {
      data->key = LV_KEY_NEXT;
      if (failReason != failReason_t::NOT_YET || status.pauseState) {
        lv_obj_scroll_by(lv_obj_get_child(screenMonitor, 1), 0, -50, LV_ANIM_ON);
      }
    }
    else if (key == '#') {
      data->key = LV_KEY_BACKSPACE;
    }
    else if (key == '*') {
      if (status.testState) {
        status.pauseState = !status.pauseState;
        if (!status.pauseState) {
          lv_obj_scroll_to(lv_obj_get_child(screenMonitor, 1), 0, 0, LV_ANIM_OFF);
        }
      }
    }
    else if (key == 'F') {
      esp_restart();
    }
    else if (key == 'G') {
      status.testState = true;
      lv_disp_load_scr(screenMonitor);
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