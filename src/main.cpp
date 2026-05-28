#include "lvgl.h"

static void event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_CLICKED) {
        LV_LOG_USER("Clicked");
    }
    else if(code == LV_EVENT_VALUE_CHANGED) {
        LV_LOG_USER("Toggled");
    }
}

// Configuration par défaut pour les zones de fréquence cardiaque
static int32_t hr_value = 98; // Correction syntaxe : "tatic" -> "static"
static int8_t hr_step = 1;
static lv_obj_t * needle_line = NULL;
static lv_obj_t * hr_value_label = NULL;
static lv_obj_t * bpm_label = NULL;
static lv_obj_t * scale = NULL;

typedef struct {
    lv_style_t items;
    lv_style_t indicator;
    lv_style_t main;
} section_styles_t;

static section_styles_t zone1_styles;
static section_styles_t zone2_styles;
static section_styles_t zone3_styles;
static section_styles_t zone4_styles;
static section_styles_t zone5_styles;

static lv_color_t get_hr_zone_color(int32_t hr)
{
    if(hr < 117) return lv_palette_main(LV_PALETTE_GREY);       /* Zone 1 */
    else if(hr < 135) return lv_palette_main(LV_PALETTE_BLUE);  /* Zone 2 */
    else if(hr < 158) return lv_palette_main(LV_PALETTE_GREEN); /* Zone 3 */
    else if(hr < 176) return lv_palette_main(LV_PALETTE_ORANGE);/* Zone 4 */
    else return lv_palette_main(LV_PALETTE_RED);                /* Zone 5 */
}

static void hr_anim_timer_cb(lv_timer_t * timer)
{
    LV_UNUSED(timer);

    hr_value += hr_step;

    if(hr_value >= 195) {
        hr_value = 195;
        hr_step = -1;
    }
    else if(hr_value <= 98) {
        hr_value = 98;
        hr_step = 1;
    }

    /* Mettre à jour l'aiguille (la longueur doit correspondre à celle définie à l'init) */
    lv_scale_set_line_needle_value(scale, needle_line, 65, hr_value);

    /* Mettre à jour le texte */
    lv_label_set_text_fmt(hr_value_label, "%d", hr_value);

    /* Ajuster la couleur du texte selon la zone active */
    lv_color_t zone_color = get_hr_zone_color(hr_value);
    lv_obj_set_style_text_color(hr_value_label, zone_color, 0);
    lv_obj_set_style_text_color(bpm_label, zone_color, 0);
}

static void init_section_styles(section_styles_t * styles, lv_color_t color)
{
    lv_style_init(&styles->items);
    lv_style_set_line_color(&styles->items, color);
    lv_style_set_line_width(&styles->items, 0);

    lv_style_init(&styles->indicator);
    lv_style_set_line_color(&styles->indicator, color);
    lv_style_set_line_width(&styles->indicator, 0);

    lv_style_init(&styles->main);
    lv_style_set_arc_color(&styles->main, color);
    lv_style_set_arc_width(&styles->main, 20);
}

static void add_section(lv_obj_t * target_scale,
                        int32_t from,
                        int32_t to,
                        section_styles_t * styles)
{
    lv_scale_section_t * sec = lv_scale_add_section(target_scale);
    lv_scale_section_set_range(sec, from, to); // Correction API v9
    lv_scale_section_set_style(sec, LV_PART_ITEMS, &styles->items);
    lv_scale_section_set_style(sec, LV_PART_INDICATOR, &styles->indicator);
    lv_scale_section_set_style(sec, LV_PART_MAIN, &styles->main);
}

void lv_scale_hr(void) // Renommé pour éviter le conflit de type avec l'objet "scale"
{
    scale = lv_scale_create(lv_screen_active());
    lv_obj_center(scale);
    lv_obj_set_size(scale, 200, 200);

    lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_range(scale, 98, 195);
    lv_scale_set_total_tick_count(scale, 15);
    lv_scale_set_major_tick_every(scale, 3);
    lv_scale_set_angle_range(scale, 280);
    lv_scale_set_rotation(scale, 130);
    lv_scale_set_label_show(scale, false);

    lv_obj_set_style_length(scale, 6, LV_PART_ITEMS);
    lv_obj_set_style_length(scale, 10, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(scale, 0, LV_PART_MAIN);

    /* Initialisation des sections colorées */
    init_section_styles(&zone1_styles, lv_palette_main(LV_PALETTE_GREY));
    add_section(scale, 98, 117, &zone1_styles);

    init_section_styles(&zone2_styles, lv_palette_main(LV_PALETTE_BLUE));
    add_section(scale, 117, 135, &zone2_styles);

    init_section_styles(&zone3_styles, lv_palette_main(LV_PALETTE_GREEN));
    add_section(scale, 135, 158, &zone3_styles);

    init_section_styles(&zone4_styles, lv_palette_main(LV_PALETTE_ORANGE));
    add_section(scale, 158, 176, &zone4_styles);

    init_section_styles(&zone5_styles, lv_palette_main(LV_PALETTE_RED));
    add_section(scale, 176, 195, &zone5_styles);

    // Création de l'aiguille
    needle_line = lv_line_create(scale);
    lv_obj_set_style_line_color(needle_line, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
    lv_obj_set_style_line_width(needle_line, 6, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);

    int32_t current_hr = 145;
    // La longueur de l'aiguille (65) doit correspondre au rayon intérieur (Scale = 200x200 -> rayon = 100)
    lv_scale_set_line_needle_value(scale, needle_line, 65, current_hr);

    // Cercle central pour masquer l'intérieur et afficher le texte
    lv_obj_t * circle = lv_obj_create(lv_screen_active());
    lv_obj_set_size(circle, 140, 140);
    lv_obj_center(circle);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(circle, lv_obj_get_style_bg_color(lv_screen_active(), LV_PART_MAIN), 0);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(circle, 0, LV_PART_MAIN);

    lv_obj_t * hr_container = lv_obj_create(circle);
    lv_obj_center(hr_container);
    lv_obj_set_size(hr_container, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(hr_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(hr_container, 0, 0);
    lv_obj_set_layout(hr_container, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(hr_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(hr_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(hr_container, 0, 0);
    lv_obj_set_flex_align(hr_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    hr_value_label = lv_label_create(hr_container);
    lv_label_set_text_fmt(hr_value_label, "%d", current_hr);
        lv_obj_set_style_text_font(hr_value_label, &lv_font_montserrat_14, 0); // Modifié de 1 à 0 (Sélecteur standard)
    lv_obj_set_style_text_align(hr_value_label, LV_TEXT_ALIGN_CENTER, 0);

    bpm_label = lv_label_create(hr_container);
    lv_label_set_text(bpm_label, "bpm");
    lv_obj_set_style_text_font(bpm_label, &lv_font_montserrat_14, 0); // Utilisation d'une police standard v8/v9
    lv_obj_set_style_text_align(bpm_label, LV_TEXT_ALIGN_CENTER, 0);

    lv_color_t zone_color = get_hr_zone_color(current_hr);
    lv_obj_set_style_text_color(hr_value_label, zone_color, 0);
    lv_obj_set_style_text_color(bpm_label, zone_color, 0);

    /* Timer pour l'animation toutes les 80 ms */
    lv_timer_create(hr_anim_timer_cb, 80, NULL);
}

void testLvgl()
{
  lv_obj_t * label;

  lv_scale_hr();
  
  lv_obj_t * btn1 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn1, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn1, LV_ALIGN_CENTER, -130, 90); // Descendu pour dégager le widget rond central
  lv_obj_remove_flag(btn1, LV_OBJ_FLAG_PRESS_LOCK);

  label = lv_label_create(btn1);
  lv_label_set_text(label, "Button");
  lv_obj_center(label);

  lv_obj_t * btn2 = lv_button_create(lv_screen_active());
  lv_obj_add_event_cb(btn2, event_handler, LV_EVENT_ALL, NULL);
  lv_obj_align(btn2, LV_ALIGN_CENTER, 130, 90); // Descendu également
  lv_obj_add_flag(btn2, LV_OBJ_FLAG_CHECKABLE);
  lv_obj_set_height(btn2, LV_SIZE_CONTENT);

  label = lv_label_create(btn2);
  lv_label_set_text(label, "Toggle");
  lv_obj_center(label);
}

#ifdef ARDUINO

#include "lvglDrivers.h"

void mySetup()
{
  testLvgl();
}

void loop()
{
}

void myTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    lv_timer_handler(); 
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(15)); 
  }
}

#else

#include "lvgl.h"
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