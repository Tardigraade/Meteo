#include "lvgl.h"
#include <stdlib.h> // Pour abs()

// Variables des capteurs
volatile static float t_interieur = 0.0;
volatile static float h_interieur = 0.0;
volatile static float t_exterieur = 0.0;
volatile static float h_exterieur = 0.0;
volatile static bool nouvelles_donnees_dispo = false;

// Variables des seuils
volatile static float seuil_h_interieur = 50.0;
volatile static float seuil_h_exterieur = 50.0;

// Objets LVGL globaux
static lv_obj_t * arc_temp_int;
static lv_obj_t * label_temp_int_val;
static lv_obj_t * label_int; // Pour l'humidité

static lv_obj_t * arc_temp_ext;
static lv_obj_t * label_temp_ext_val;
static lv_obj_t * label_ext; // Pour l'humidité

static lv_obj_t * label_seuil_int;
static lv_obj_t * label_seuil_ext;
static lv_obj_t * alerte_int;
static lv_obj_t * alerte_ext;

// Importation des mutex
extern bool lvglLock(TickType_t xBlockTime);
extern bool lvglUnlock();

// --- Callbacks pour les sliders d'humidité ---
static void slider_int_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    seuil_h_interieur = (float)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label_seuil_int, "Seuil Hum : %d%%", (int)seuil_h_interieur);
}

static void slider_ext_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    seuil_h_exterieur = (float)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label_seuil_ext, "Seuil Hum : %d%%", (int)seuil_h_exterieur);
}

void testLvgl(void)
{
    static lv_style_t style_carre;
    lv_style_init(&style_carre);
    lv_style_set_bg_opa(&style_carre, LV_OPA_COVER);
    lv_style_set_border_color(&style_carre, lv_color_hex(0x000000));
    lv_style_set_border_width(&style_carre, 4);
    lv_style_set_radius(&style_carre, 10);

    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(screen, 8, 0);
    lv_obj_set_style_pad_column(screen, 8, 0);

    // ================= CARTE INTERIEUR =================
    lv_obj_t * card_interieur = lv_obj_create(screen);
    lv_obj_set_flex_grow(card_interieur, 1);
    lv_obj_set_size(card_interieur, LV_PCT(50), LV_PCT(100));
    lv_obj_add_style(card_interieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_interieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_interieur, lv_palette_main(LV_PALETTE_GREEN), 0);
    
    lv_obj_set_flex_flow(card_interieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_interieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * titre_int = lv_label_create(card_interieur);
    lv_label_set_text(titre_int, "Interieur");
    
    // ARC DE TEMPERATURE INTERIEUR
    arc_temp_int = lv_arc_create(card_interieur);
    lv_obj_set_size(arc_temp_int, 100, 100);
    lv_arc_set_rotation(arc_temp_int, 135);
    lv_arc_set_bg_angles(arc_temp_int, 0, 270);
    lv_arc_set_range(arc_temp_int, -10, 50); // Plage de température en Celsius (-10 à +50)
    lv_obj_remove_flag(arc_temp_int, LV_OBJ_FLAG_CLICKABLE); // Rendre non-cliquable
    // Masquer le bouton (knob) pour faire un anneau propre
    lv_obj_set_style_bg_opa(arc_temp_int, 0, LV_PART_KNOB); 
    lv_obj_set_style_border_opa(arc_temp_int, 0, LV_PART_KNOB);
    lv_arc_set_value(arc_temp_int, 0);

    // Label centré dans l'arc
    label_temp_int_val = lv_label_create(arc_temp_int);
    lv_obj_center(label_temp_int_val);
    lv_label_set_text(label_temp_int_val, "--.-°C");

    // Humidité
    label_int = lv_label_create(card_interieur);
    lv_label_set_text(label_int, "Hum: --.-%");

    // Slider Humidité
    lv_obj_t * slider_int = lv_slider_create(card_interieur);
    lv_slider_set_range(slider_int, 0, 100);
    lv_slider_set_value(slider_int, (int)seuil_h_interieur, LV_ANIM_OFF);
    lv_obj_set_width(slider_int, LV_PCT(90));
    lv_obj_add_event_cb(slider_int, slider_int_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    label_seuil_int = lv_label_create(card_interieur);
    lv_label_set_text_fmt(label_seuil_int, "Seuil Hum : %d%%", (int)seuil_h_interieur);

    alerte_int = lv_label_create(card_interieur);
    lv_label_set_text(alerte_int, LV_SYMBOL_WARNING " HUMIDITE HAUTE !");
    lv_obj_set_style_text_color(alerte_int, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_flag(alerte_int, LV_OBJ_FLAG_HIDDEN); 

    // ================= CARTE EXTERIEUR =================
    lv_obj_t * card_exterieur = lv_obj_create(screen);
    lv_obj_set_flex_grow(card_exterieur, 1);
    lv_obj_set_size(card_exterieur, LV_PCT(50), LV_PCT(100));
    lv_obj_add_style(card_exterieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_exterieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_exterieur, lv_palette_main(LV_PALETTE_RED), 0);
    
    lv_obj_set_flex_flow(card_exterieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_exterieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t * titre_ext = lv_label_create(card_exterieur);
    lv_label_set_text(titre_ext, "Exterieur");

    // ARC DE TEMPERATURE EXTERIEUR
    arc_temp_ext = lv_arc_create(card_exterieur);
    lv_obj_set_size(arc_temp_ext, 100, 100);
    lv_arc_set_rotation(arc_temp_ext, 135);
    lv_arc_set_bg_angles(arc_temp_ext, 0, 270);
    lv_arc_set_range(arc_temp_ext, -10, 50); // Plage -10 à +50
    lv_obj_remove_flag(arc_temp_ext, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_bg_opa(arc_temp_ext, 0, LV_PART_KNOB); 
    lv_obj_set_style_border_opa(arc_temp_ext, 0, LV_PART_KNOB);
    lv_arc_set_value(arc_temp_ext, 0);

    label_temp_ext_val = lv_label_create(arc_temp_ext);
    lv_obj_center(label_temp_ext_val);
    lv_label_set_text(label_temp_ext_val, "--.-°C");

    // Humidité
    label_ext = lv_label_create(card_exterieur);
    lv_label_set_text(label_ext, "Hum: --.-%");

    // Slider Humidité
    lv_obj_t * slider_ext = lv_slider_create(card_exterieur);
    lv_slider_set_range(slider_ext, 0, 100);
    lv_slider_set_value(slider_ext, (int)seuil_h_exterieur, LV_ANIM_OFF);
    lv_obj_set_width(slider_ext, LV_PCT(90));
    lv_obj_add_event_cb(slider_ext, slider_ext_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    label_seuil_ext = lv_label_create(card_exterieur);
    lv_label_set_text_fmt(label_seuil_ext, "Seuil Hum : %d%%", (int)seuil_h_exterieur);

    alerte_ext = lv_label_create(card_exterieur);
    lv_label_set_text(alerte_ext, LV_SYMBOL_WARNING " HUMIDITE HAUTE !");
    lv_obj_set_style_text_color(alerte_ext, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_set_style_bg_color(alerte_ext, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(alerte_ext, LV_OPA_COVER, 0);
    lv_obj_add_flag(alerte_ext, LV_OBJ_FLAG_HIDDEN);
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
        } else {
            Serial.println("Erreur Capteur 1 D2");
        }
        
        if (!isnan(h2) && !isnan(t2)) {
            t_exterieur = t2;
            h_exterieur = h2;
            change = true;
        } else {
            Serial.println("Erreur Capteur 2 D4");
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
            
            // --- Mise à jour Intérieur ---
            if (arc_temp_int != NULL) {
                // Met à jour la barre de progression (limité aux valeurs entières)
                lv_arc_set_value(arc_temp_int, (int)t_interieur);
                
                // Met à jour le texte au centre
                lv_label_set_text_fmt(label_temp_int_val, "%d.%d°C", 
                    (int)t_interieur, 
                    abs((int)(t_interieur * 10) % 10)); // abs() pour éviter -5.-2°C
            }
            if (label_int != NULL) {
                lv_label_set_text_fmt(label_int, "Hum: %d.%d%%",
                    (int)h_interieur, abs((int)(h_interieur * 10) % 10));
            }
            
            if (h_interieur > seuil_h_interieur) {
                lv_obj_remove_flag(alerte_int, LV_OBJ_FLAG_HIDDEN); 
            } else {
                lv_obj_add_flag(alerte_int, LV_OBJ_FLAG_HIDDEN);    
            }
            
            // --- Mise à jour Extérieur ---
            if (arc_temp_ext != NULL) {
                lv_arc_set_value(arc_temp_ext, (int)t_exterieur);
                
                lv_label_set_text_fmt(label_temp_ext_val, "%d.%d°C", 
                    (int)t_exterieur, 
                    abs((int)(t_exterieur * 10) % 10));
            }
            if (label_ext != NULL) {
                lv_label_set_text_fmt(label_ext, "Hum: %d.%d%%",
                    (int)h_exterieur, abs((int)(h_exterieur * 10) % 10));
            }
            
            if (h_exterieur > seuil_h_exterieur) {
                lv_obj_remove_flag(alerte_ext, LV_OBJ_FLAG_HIDDEN); 
            } else {
                lv_obj_add_flag(alerte_ext, LV_OBJ_FLAG_HIDDEN);    
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