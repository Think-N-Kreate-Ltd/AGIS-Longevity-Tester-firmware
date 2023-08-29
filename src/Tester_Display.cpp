#include <Tester_Display.h>
#include <Keypad.h>

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf[TFT_WIDTH * TFT_HEIGHT / 10];

TFT_eSPI tft = TFT_eSPI();

lv_group_t * grp;           /*a group to group all keypad evented object*/
lv_indev_t * keypad_indev;  /*a driver in LVGL and save the created input device object*/

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
  lv_group_set_default(grp);  /*let the object create added to this group*/
  lv_indev_set_group(keypad_indev, grp);
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
      data->key = key;  /*should not enter here*/
    }

    data->state = LV_INDEV_STATE_PRESSED;
  }
  else if (keypad.getState() == 0) {  // when keypad pressing is released
    data->state = LV_INDEV_STATE_RELEASED;
  }
}