//-----------------------------------------------------------------------------------
//
// Distributed under MIT Licence
//   See https://github.com/Blackboard_Examples/blob/main/LICENSE
//
//-----------------------------------------------------------------------------------
//
// Read the potentiometer rotation value via the XADC IP using FreeRTOS. Example 1c.
//
//-----------------------------------------------------------------------------------

#include "ui.h"
#include "lvgl.h"

static lv_obj_t* g_arc;
static lv_obj_t* g_percent_label;
static lv_obj_t* g_raw_label;

void UI_Init(void) {
  lv_obj_t* screen = lv_screen_active();
  lv_obj_clean(screen);

  lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B1020), 0);
  lv_obj_set_style_bg_grad_color(screen, lv_color_hex(0x121A33), 0);
  lv_obj_set_style_bg_grad_dir(screen, LV_GRAD_DIR_VER, 0);

  lv_obj_t* title = lv_label_create(screen);
  lv_label_set_text(title, "Blackboard Potentiometer");
  lv_obj_set_style_text_color(title, lv_color_hex(0xE6ECFF), 0);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
  lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 28);

  g_arc = lv_arc_create(screen);
  lv_obj_set_size(g_arc, 360, 360);
  lv_obj_center(g_arc);
  lv_arc_set_range(g_arc, 0, 100);
  lv_arc_set_rotation(g_arc, 135);
  lv_arc_set_bg_angles(g_arc, 0, 270);
  lv_arc_set_value(g_arc, 0);
  lv_obj_remove_flag(g_arc, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_style_arc_width(g_arc, 26, LV_PART_MAIN);
  lv_obj_set_style_arc_width(g_arc, 30, LV_PART_INDICATOR);
  lv_obj_set_style_arc_color(g_arc, lv_color_hex(0x1E294A), LV_PART_MAIN);
  lv_obj_set_style_arc_color(g_arc, lv_color_hex(0x44D7B6), LV_PART_INDICATOR);
  lv_obj_set_style_opa(g_arc, LV_OPA_TRANSP, LV_PART_KNOB);

  g_percent_label = lv_label_create(screen);
  lv_label_set_text(g_percent_label, "0%");
  lv_obj_set_style_text_color(g_percent_label, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(g_percent_label, &lv_font_montserrat_48, 0);
  lv_obj_center(g_percent_label);

  g_raw_label = lv_label_create(screen);
  lv_label_set_text(g_raw_label, "raw 0");
  lv_obj_set_style_text_color(g_raw_label, lv_color_hex(0x95A2C6), 0);
  lv_obj_set_style_text_font(g_raw_label, &lv_font_montserrat_20, 0);
  lv_obj_align(g_raw_label, LV_ALIGN_CENTER, 0, 84);
}

void UI_SetPotValue(uint16_t raw, uint32_t percent) {
  lv_arc_set_value(g_arc, (int32_t)percent);
  lv_label_set_text_fmt(g_percent_label, "%lu%%", (unsigned long)percent);
  lv_label_set_text_fmt(g_raw_label, "raw %u", raw);
}
