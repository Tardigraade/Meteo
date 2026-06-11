#include "lvgl.h"
#include <stdlib.h> 

// Variables des capteurs
volatile static float t_interieur = 0.0;
volatile static float h_interieur = 0.0;
volatile static float t_exterieur = 0.0;
volatile static float h_exterieur = 0.0;
volatile static bool nouvelles_donnees_dispo = false;

// Variables des seuils (Valeurs par défaut)
volatile static float t_int_min = 18.0, t_int_max = 25.0;
volatile static float h_int_min = 40.0, h_int_max = 60.0;

volatile static float t_ext_min = 0.0,  t_ext_max = 35.0;
volatile static float h_ext_min = 30.0, h_ext_max = 70.0;

// Objets LVGL globaux (Dashboard)
static lv_obj_t * arc_temp_int;
static lv_obj_t * label_temp_int_val;
static lv_obj_t * label_int; 
static lv_obj_t * alerte_t_int;
static lv_obj_t * alerte_h_int;

static lv_obj_t * arc_temp_ext;
static lv_obj_t * label_temp_ext_val;
static lv_obj_t * label_ext; 
static lv_obj_t * alerte_t_ext;
static lv_obj_t * alerte_h_ext;

// Objets LVGL globaux (Labels des seuils)
static lv_obj_t * lbl_cfg_t_int_min; static lv_obj_t * lbl_cfg_t_int_max;
static lv_obj_t * lbl_cfg_h_int_min; static lv_obj_t * lbl_cfg_h_int_max;

static lv_obj_t * lbl_cfg_t_ext_min; static lv_obj_t * lbl_cfg_t_ext_max;
static lv_obj_t * lbl_cfg_h_ext_min; static lv_obj_t * lbl_cfg_h_ext_max;

// Importation des mutex
extern bool lvglLock(TickType_t xBlockTime);
extern bool lvglUnlock();

// CALLBACKS DES SLIDERS

//  Intérieur 
static void cb_t_int_min(lv_event_t * e) {
    t_int_min = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_t_int_min, "Temp Min: %d°C", (int)t_int_min);
}
static void cb_t_int_max(lv_event_t * e) {
    t_int_max = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_t_int_max, "Temp Max: %d°C", (int)t_int_max);
}
static void cb_h_int_min(lv_event_t * e) {
    h_int_min = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_h_int_min, "Hum Min: %d%%", (int)h_int_min);
}
static void cb_h_int_max(lv_event_t * e) {
    h_int_max = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_h_int_max, "Hum Max: %d%%", (int)h_int_max);
}

// -- Extérieur --
static void cb_t_ext_min(lv_event_t * e) {
    t_ext_min = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_t_ext_min, "Temp Min: %d°C", (int)t_ext_min);
}
static void cb_t_ext_max(lv_event_t * e) {
    t_ext_max = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_t_ext_max, "Temp Max: %d°C", (int)t_ext_max);
}
static void cb_h_ext_min(lv_event_t * e) {
    h_ext_min = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_h_ext_min, "Hum Min: %d%%", (int)h_ext_min);
}
static void cb_h_ext_max(lv_event_t * e) {
    h_ext_max = (float)lv_slider_get_value((lv_obj_t *)lv_event_get_target(e));
    lv_label_set_text_fmt(lbl_cfg_h_ext_max, "Hum Max: %d%%", (int)h_ext_max);
}

// Fonction utilitaire pour générer les sliders de réglage
static void creer_slider_reglage(lv_obj_t * parent, const char * titre, int min_val, int max_val, int def_val, lv_obj_t ** label_ref, lv_event_cb_t cb) {
    *label_ref = lv_label_create(parent);
    lv_label_set_text_fmt(*label_ref, "%s: %d", titre, def_val);
    
    lv_obj_t * slider = lv_slider_create(parent);
    lv_slider_set_range(slider, min_val, max_val);
    lv_slider_set_value(slider, def_val, LV_ANIM_OFF);
    lv_obj_set_width(slider, LV_PCT(90));
    lv_obj_add_event_cb(slider, cb, LV_EVENT_VALUE_CHANGED, NULL);
}

void testLvgl(void)
{
    lv_obj_t * screen = lv_screen_active();
    
    // Création du système d'onglets
    lv_obj_t * tabview = lv_tabview_create(screen);
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_TOP);
    
    lv_obj_t * tab_dash = lv_tabview_add_tab(tabview, "Dashboard");
    lv_obj_t * tab_cfg_int = lv_tabview_add_tab(tabview, "Seuils Int");
    lv_obj_t * tab_cfg_ext = lv_tabview_add_tab(tabview, "Seuils Ext");

    static lv_style_t style_carre;
    lv_style_init(&style_carre);
    lv_style_set_bg_opa(&style_carre, LV_OPA_COVER);
    lv_style_set_border_color(&style_carre, lv_color_hex(0x000000));
    lv_style_set_border_width(&style_carre, 4);
    lv_style_set_radius(&style_carre, 10);

    //  DASHBOARD 
    lv_obj_set_flex_flow(tab_dash, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(tab_dash, 4, 0);

    // --- Carte Interieur ---
    lv_obj_t * card_interieur = lv_obj_create(tab_dash);
    lv_obj_set_flex_grow(card_interieur, 1);
    lv_obj_set_height(card_interieur, LV_PCT(100));
    lv_obj_add_style(card_interieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_interieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_interieur, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_obj_set_flex_flow(card_interieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_interieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * titre_int = lv_label_create(card_interieur);
    lv_label_set_text(titre_int, "Interieur");
    
    arc_temp_int = lv_arc_create(card_interieur);
    lv_obj_set_size(arc_temp_int, 90, 90);
    lv_arc_set_rotation(arc_temp_int, 135);
    lv_arc_set_bg_angles(arc_temp_int, 0, 270);
    lv_arc_set_range(arc_temp_int, -10, 50);
    lv_obj_remove_flag(arc_temp_int, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(arc_temp_int, 0, LV_PART_KNOB); 
    lv_obj_set_style_border_opa(arc_temp_int, 0, LV_PART_KNOB);
    
    label_temp_int_val = lv_label_create(arc_temp_int);
    lv_obj_center(label_temp_int_val);
    lv_label_set_text(label_temp_int_val, "--.-°C");

    label_int = lv_label_create(card_interieur);
    lv_label_set_text(label_int, "Hum: --.-%");

    alerte_t_int = lv_label_create(card_interieur);
    lv_obj_set_style_text_color(alerte_t_int, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_flag(alerte_t_int, LV_OBJ_FLAG_HIDDEN); 

    alerte_h_int = lv_label_create(card_interieur);
    lv_obj_set_style_text_color(alerte_h_int, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_flag(alerte_h_int, LV_OBJ_FLAG_HIDDEN); 

    // Carte Exterieur 
    lv_obj_t * card_exterieur = lv_obj_create(tab_dash);
    lv_obj_set_flex_grow(card_exterieur, 1);
    lv_obj_set_height(card_exterieur, LV_PCT(100));
    lv_obj_add_style(card_exterieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_exterieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_exterieur, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_flex_flow(card_exterieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_exterieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * titre_ext = lv_label_create(card_exterieur);
    lv_label_set_text(titre_ext, "Exterieur");

    arc_temp_ext = lv_arc_create(card_exterieur);
    lv_obj_set_size(arc_temp_ext, 90, 90);
    lv_arc_set_rotation(arc_temp_ext, 135);
    lv_arc_set_bg_angles(arc_temp_ext, 0, 270);
    lv_arc_set_range(arc_temp_ext, -10, 50);
    lv_obj_remove_flag(arc_temp_ext, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(arc_temp_ext, 0, LV_PART_KNOB); 
    lv_obj_set_style_border_opa(arc_temp_ext, 0, LV_PART_KNOB);
    
    label_temp_ext_val = lv_label_create(arc_temp_ext);
    lv_obj_center(label_temp_ext_val);
    lv_label_set_text(label_temp_ext_val, "--.-°C");

    label_ext = lv_label_create(card_exterieur);
    lv_label_set_text(label_ext, "Hum: --.-%");

    alerte_t_ext = lv_label_create(card_exterieur);
    lv_obj_set_style_text_color(alerte_t_ext, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_add_flag(alerte_t_ext, LV_OBJ_FLAG_HIDDEN); 

    alerte_h_ext = lv_label_create(card_exterieur);
    lv_obj_set_style_text_color(alerte_h_ext, lv_color_hex(0xFFFFFF), 0); 
    lv_obj_add_flag(alerte_h_ext, LV_OBJ_FLAG_HIDDEN); 

    //  REGLAGES INTERIEURS 
    lv_obj_set_flex_flow(tab_cfg_int, LV_FLEX_FLOW_COLUMN);
    creer_slider_reglage(tab_cfg_int, "Temp Min", -10, 50, (int)t_int_min, &lbl_cfg_t_int_min, cb_t_int_min);
    creer_slider_reglage(tab_cfg_int, "Temp Max", -10, 50, (int)t_int_max, &lbl_cfg_t_int_max, cb_t_int_max);
    creer_slider_reglage(tab_cfg_int, "Hum Min", 0, 100, (int)h_int_min, &lbl_cfg_h_int_min, cb_h_int_min);
    creer_slider_reglage(tab_cfg_int, "Hum Max", 0, 100, (int)h_int_max, &lbl_cfg_h_int_max, cb_h_int_max);

    //  REGLAGES EXTERIEURS 
    lv_obj_set_flex_flow(tab_cfg_ext, LV_FLEX_FLOW_COLUMN);
    creer_slider_reglage(tab_cfg_ext, "Temp Min", -10, 50, (int)t_ext_min, &lbl_cfg_t_ext_min, cb_t_ext_min);
    creer_slider_reglage(tab_cfg_ext, "Temp Max", -10, 50, (int)t_ext_max, &lbl_cfg_t_ext_max, cb_t_ext_max);
    creer_slider_reglage(tab_cfg_ext, "Hum Min", 0, 100, (int)h_ext_min, &lbl_cfg_h_ext_min, cb_h_ext_min);
    creer_slider_reglage(tab_cfg_ext, "Hum Max", 0, 100, (int)h_ext_max, &lbl_cfg_h_ext_max, cb_h_ext_max);
}

#define ARDUINO  

#ifdef ARDUINO

#include "lvglDrivers.h"
#include "DHT.h"

#define DHTPIN D2
#define DHTPIN2 D4
#define DHTTYPE DHT22     
DHT dht(DHTPIN, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

unsigned long lastDHTRead = 0;
const unsigned long dhtInterval = 2000;

void mySetup()
{
  testLvgl();
  Serial.begin(115200);
  dht.begin();
  dht2.begin();
  
  xTaskCreate(
    myTask,          
    "LVGL Update Task",     
    4096,            
    NULL,           
    3,             
    NULL);
  Serial.println("Setup done");
}

void loop()
{
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastDHTRead >= dhtInterval) {
        lastDHTRead = currentMillis;
        
        float h2 = dht2.readHumidity();
        float t2 = dht2.readTemperature();
        
        float h = dht.readHumidity();
        float t = dht.readTemperature();

        bool change = false;

        if (!isnan(h) && !isnan(t)) {
            t_interieur = t;
            h_interieur = h;
            change = true;
        }
        
        if (!isnan(h2) && !isnan(t2)) {
            t_exterieur = t2;
            h_exterieur = h2;
            change = true;
        }

        if (change) {
            nouvelles_donnees_dispo = true;
        }
    }
}

void myTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    if (nouvelles_donnees_dispo) {
        
        if (lvglLock(pdMS_TO_TICKS(100))) {
            
            // MISE A JOUR INTERIEUR 
            if (arc_temp_int != NULL) {
                lv_arc_set_value(arc_temp_int, (int)t_interieur);
                lv_label_set_text_fmt(label_temp_int_val, "%d.%d°C", (int)t_interieur, abs((int)(t_interieur * 10) % 10));
            }
            if (label_int != NULL) {
                lv_label_set_text_fmt(label_int, "Hum: %d.%d%%", (int)h_interieur, abs((int)(h_interieur * 10) % 10));
            }
            
            // Alertes Température Intérieure
            if (t_interieur > t_int_max) {
                lv_label_set_text(alerte_t_int, LV_SYMBOL_WARNING " TEMP HAUTE");
                lv_obj_remove_flag(alerte_t_int, LV_OBJ_FLAG_HIDDEN);
            } else if (t_interieur < t_int_min) {
                lv_label_set_text(alerte_t_int, LV_SYMBOL_WARNING " TEMP BASSE");
                lv_obj_remove_flag(alerte_t_int, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(alerte_t_int, LV_OBJ_FLAG_HIDDEN);
            }

            // Alertes Humidité Intérieure
            if (h_interieur > h_int_max) {
                lv_label_set_text(alerte_h_int, LV_SYMBOL_WARNING " HUM HAUTE");
                lv_obj_remove_flag(alerte_h_int, LV_OBJ_FLAG_HIDDEN);
            } else if (h_interieur < h_int_min) {
                lv_label_set_text(alerte_h_int, LV_SYMBOL_WARNING " HUM BASSE");
                lv_obj_remove_flag(alerte_h_int, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(alerte_h_int, LV_OBJ_FLAG_HIDDEN);
            }

            // MISE A JOUR EXTERIEUR 
            if (arc_temp_ext != NULL) {
                lv_arc_set_value(arc_temp_ext, (int)t_exterieur);
                lv_label_set_text_fmt(label_temp_ext_val, "%d.%d°C", (int)t_exterieur, abs((int)(t_exterieur * 10) % 10));
            }
            if (label_ext != NULL) {
                lv_label_set_text_fmt(label_ext, "Hum: %d.%d%%", (int)h_exterieur, abs((int)(h_exterieur * 10) % 10));
            }
            
            // Alertes Température Extérieure
            if (t_exterieur > t_ext_max) {
                lv_label_set_text(alerte_t_ext, LV_SYMBOL_WARNING " TEMP HAUTE");
                lv_obj_remove_flag(alerte_t_ext, LV_OBJ_FLAG_HIDDEN);
            } else if (t_exterieur < t_ext_min) {
                lv_label_set_text(alerte_t_ext, LV_SYMBOL_WARNING " TEMP BASSE");
                lv_obj_remove_flag(alerte_t_ext, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(alerte_t_ext, LV_OBJ_FLAG_HIDDEN);
            }

            // Alertes Humidité Extérieure
            if (h_exterieur > h_ext_max) {
                lv_label_set_text(alerte_h_ext, LV_SYMBOL_WARNING " HUM HAUTE");
                lv_obj_remove_flag(alerte_h_ext, LV_OBJ_FLAG_HIDDEN);
            } else if (h_exterieur < h_ext_min) {
                lv_label_set_text(alerte_h_ext, LV_SYMBOL_WARNING " HUM BASSE");
                lv_obj_remove_flag(alerte_h_ext, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(alerte_h_ext, LV_OBJ_FLAG_HIDDEN);
            }
            
            lvglUnlock();
        }
        nouvelles_donnees_dispo = false; 
    }
    
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50)); 
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