#include "lvgl.h"

//  Variables Globales 
static lv_obj_t * status_label;

// Événements des Boutons 
static void btn_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Button Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Button Toggled");
    }
}

//  Événement du Switch 
static void switch_event_cb(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target_obj(e);
    bool on = lv_obj_has_state(sw, LV_STATE_CHECKED);
    
    LV_LOG_USER("switch %s", on ? "on" : "off");
    lv_label_set_text(status_label, on ? "Etat: ON" : "Etat: OFF");
}

//  gauge 
void lv_example_scale_4(lv_obj_t * parent)
{
    lv_obj_t * scale = lv_scale_create(parent);
    lv_obj_set_size(scale, 150, 150);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
    lv_obj_center(scale);

    lv_scale_set_total_tick_count(scale, 21);
    lv_scale_set_major_tick_every(scale, 5);

    lv_obj_set_style_length(scale, 5, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_scale_set_range(scale, 0, 100);

    static const char * custom_labels[] = {"0 °C", "25 °C", "50 °C", "75 °C", "100 °C", NULL};
    lv_scale_set_text_src(scale, custom_labels);

    static lv_style_t indicator_style;
    lv_style_init(&indicator_style);

    /* Label style properties */
    lv_style_set_text_font(&indicator_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));

    /* Major tick properties */
    lv_style_set_line_color(&indicator_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_width(&indicator_style, 10U);      /*Tick length*/
    lv_style_set_line_width(&indicator_style, 2U);  /*Tick width*/
    lv_obj_add_style(scale, &indicator_style, LV_PART_INDICATOR);

    static lv_style_t minor_ticks_style;
    lv_style_init(&minor_ticks_style);
    lv_style_set_line_color(&minor_ticks_style, lv_palette_lighten(LV_PALETTE_BLUE, 2));
    lv_style_set_width(&minor_ticks_style, 5U);         /*Tick length*/
    lv_style_set_line_width(&minor_ticks_style, 2U);    /*Tick width*/
    lv_obj_add_style(scale, &minor_ticks_style, LV_PART_ITEMS);

    static lv_style_t main_line_style;
    lv_style_init(&main_line_style);
    /* Main line properties */
    lv_style_set_arc_color(&main_line_style, lv_palette_darken(LV_PALETTE_BLUE, 3));
    lv_style_set_arc_width(&main_line_style, 2U); /*Tick width*/
    lv_obj_add_style(scale, &main_line_style, LV_PART_MAIN);

    /* Add a section */
    static lv_style_t section_minor_tick_style;
    static lv_style_t section_label_style;
    static lv_style_t section_main_line_style;

    lv_style_init(&section_label_style);
    lv_style_init(&section_minor_tick_style);
    lv_style_init(&section_main_line_style);

    /* Label style properties */
    lv_style_set_text_font(&section_label_style, LV_FONT_DEFAULT);
    lv_style_set_text_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));

    lv_style_set_line_color(&section_label_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_line_width(&section_label_style, 5U); /*Tick width*/

    lv_style_set_line_color(&section_minor_tick_style, lv_palette_lighten(LV_PALETTE_RED, 2));
    lv_style_set_line_width(&section_minor_tick_style, 4U); /*Tick width*/

    /* Main line properties */
    lv_style_set_arc_color(&section_main_line_style, lv_palette_darken(LV_PALETTE_RED, 3));
    lv_style_set_arc_width(&section_main_line_style, 4U); /*Tick width*/

    /* Configure section styles */
    lv_scale_section_t * section = lv_scale_add_section(scale);
    lv_scale_section_set_range(section, 75, 100);
    lv_scale_section_set_style(section, LV_PART_INDICATOR, &section_label_style);
    lv_scale_section_set_style(section, LV_PART_ITEMS, &section_minor_tick_style);
    lv_scale_section_set_style(section, LV_PART_MAIN, &section_main_line_style);
}

//interface graphique
void testLvgl(void)
{
    lv_obj_t * label;
    lv_obj_t * scr = lv_screen_active();

    // Configuration de l'écran en mode colonne (Flex)
    lv_obj_set_flex_flow(scr, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_cross_place(scr, LV_FLEX_ALIGN_CENTER, 0); // Centre horizontalement
    lv_obj_set_style_flex_main_place(scr, LV_FLEX_ALIGN_CENTER, 0);  // Centre verticalement
    lv_obj_set_style_pad_row(scr, 20, 20); // Espace de 20 pixels entre chaque élément

    // Bouton 1
    lv_obj_t * btn1 = lv_button_create(scr);
    lv_obj_add_event_cb(btn1, btn_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);
    label = lv_label_create(btn1);
    lv_label_set_text(label, "Button");
    lv_obj_center(label);

    //  Bouton 2
    lv_obj_t * btn2 = lv_button_create(scr);
    lv_obj_add_event_cb(btn2, btn_event_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
    lv_obj_set_height(btn2, LV_SIZE_CONTENT);
    label = lv_label_create(btn2);
    lv_label_set_text(label, "Toggle");
    lv_obj_center(label);

    //  Switch 
    lv_obj_t * sw = lv_switch_create(scr);
    lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Label du Switch
    status_label = lv_label_create(scr);
    lv_label_set_text(status_label, "State: OFF");
    // gauge //lv_example_scale_4(scr);
    lv_example_scale_4(scr);
}

#ifdef ARDUINO

#include "lvglDrivers.h"

void mySetup()
{
  testLvgl();
}

void loop()
{
  // Inactif
}

void myTask(void *pvParameters)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); 
  }
}

#else

#include "app_hal.h"
#include <cstdio>

int main(void)
{
  printf("LVGL Simulator\n");
  fflush(stdout);

  lv_init();
  hal_setup();

  testLvgl();

  hal_loop();
  return 0;
}

#endif