#include "lvgl.h"

// Variables des capteurs
volatile static float t_interieur = 0.0;
volatile static float h_interieur = 0.0;
volatile static float t_exterieur = 0.0;
volatile static float h_exterieur = 0.0;
volatile static bool nouvelles_donnees_dispo = false;

// Variables des seuils (par défaut à 50%)
volatile static float seuil_h_interieur = 50.0;
volatile static float seuil_h_exterieur = 50.0;

// Objets LVGL globaux
static lv_obj_t * label_int;
static lv_obj_t * label_ext; 
static lv_obj_t * label_seuil_int;
static lv_obj_t * label_seuil_ext;
static lv_obj_t * alerte_int;
static lv_obj_t * alerte_ext;

// Importation des mutex définis dans lvlgdriver.cpp
extern bool lvglLock(TickType_t xBlockTime);
extern bool lvglUnlock();

// --- Callbacks pour les sliders ---
static void slider_int_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    seuil_h_interieur = (float)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label_seuil_int, "Seuil : %d%%", (int)seuil_h_interieur);
}

static void slider_ext_event_cb(lv_event_t * e) {
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    seuil_h_exterieur = (float)lv_slider_get_value(slider);
    lv_label_set_text_fmt(label_seuil_ext, "Seuil : %d%%", (int)seuil_h_exterieur);
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
    
    // Organisation interne en colonne
    lv_obj_set_flex_flow(card_interieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_interieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label_int = lv_label_create(card_interieur);
    lv_label_set_text(label_int, "Interieur :\nTemp: --.-C\nHum: --.-%");

    // Slider
    lv_obj_t * slider_int = lv_slider_create(card_interieur);
    lv_slider_set_range(slider_int, 0, 100);
    lv_slider_set_value(slider_int, (int)seuil_h_interieur, LV_ANIM_OFF);
    lv_obj_set_width(slider_int, LV_PCT(90));
    lv_obj_add_event_cb(slider_int, slider_int_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Label du seuil
    label_seuil_int = lv_label_create(card_interieur);
    lv_label_set_text_fmt(label_seuil_int, "Seuil : %d%%", (int)seuil_h_interieur);

    // Label d'alerte (Caché par défaut)
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
    lv_obj_set_style_bg_color(card_exterieur, lv_palette_main(LV_PALETTE_RED), 0); // Tu peux changer en bleu si tu veux !
    
    // Organisation interne en colonne
    lv_obj_set_flex_flow(card_exterieur, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(card_exterieur, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label_ext = lv_label_create(card_exterieur);
    lv_label_set_text(label_ext, "Exterieur :\nTemp: --.-C\nHum: --.-%");

    // Slider
    lv_obj_t * slider_ext = lv_slider_create(card_exterieur);
    lv_slider_set_range(slider_ext, 0, 100);
    lv_slider_set_value(slider_ext, (int)seuil_h_exterieur, LV_ANIM_OFF);
    lv_obj_set_width(slider_ext, LV_PCT(90));
    lv_obj_add_event_cb(slider_ext, slider_ext_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Label du seuil
    label_seuil_ext = lv_label_create(card_exterieur);
    lv_label_set_text_fmt(label_seuil_ext, "Seuil : %d%%", (int)seuil_h_exterieur);

    // Label d'alerte (Caché par défaut)
    alerte_ext = lv_label_create(card_exterieur);
    lv_label_set_text(alerte_ext, LV_SYMBOL_WARNING " HUMIDITE HAUTE !");
    lv_obj_set_style_text_color(alerte_ext, lv_palette_main(LV_PALETTE_RED), 0);
    // Optionnel: fond blanc pour que ça ressorte sur la carte rouge
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
  
  // Créer la tâche pour LVGL
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
            Serial.print("Interieur -> Temp: ");
            Serial.print(t_interieur);
            Serial.print(" °C | Hum: ");
            Serial.print(h_interieur);
            Serial.println(" %");
        } else {
            Serial.println("Erreur Capteur 1 D2");
        }
        
        if (!isnan(h2) && !isnan(t2)) {
            t_exterieur = t2;
            h_exterieur = h2;
            change = true;
            Serial.print("Exterieur -> Temp: ");
            Serial.print(t_exterieur);
            Serial.print(" °C | Hum: ");
            Serial.print(h_exterieur);
            Serial.println(" %");
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
        
        // PROTECTION MUTEX : indispensable pour ne pas crasher avec l'autre tâche LVGL
        if (lvglLock(pdMS_TO_TICKS(100))) {
            
            if (label_int != NULL) {
                lv_label_set_text_fmt(label_int, "Interieur :\nTemp: %d.%dC\nHum: %d.%d%%",
                    (int)t_interieur, (int)(t_interieur * 10) % 10,
                    (int)h_interieur, (int)(h_interieur * 10) % 10
                );
                
                // Logique d'alerte Intérieure
                if (h_interieur > seuil_h_interieur) {
                    lv_obj_remove_flag(alerte_int, LV_OBJ_FLAG_HIDDEN); // Afficher
                } else {
                    lv_obj_add_flag(alerte_int, LV_OBJ_FLAG_HIDDEN);    // Cacher
                }
            }
            
            if (label_ext != NULL) {
                lv_label_set_text_fmt(label_ext, "Exterieur :\nTemp: %d.%dC\nHum: %d.%d%%",
                    (int)t_exterieur, (int)(t_exterieur * 10) % 10,
                    (int)h_exterieur, (int)(h_exterieur * 10) % 10
                );
                
                // Logique d'alerte Extérieure
                if (h_exterieur > seuil_h_exterieur) {
                    lv_obj_remove_flag(alerte_ext, LV_OBJ_FLAG_HIDDEN); // Afficher
                } else {
                    lv_obj_add_flag(alerte_ext, LV_OBJ_FLAG_HIDDEN);    // Cacher
                }
            }
            
            lvglUnlock(); // Libérer le mutex
        }
        nouvelles_donnees_dispo = false; 
    }
    
    // IMPORTANT : J'ai retiré lv_timer_handler() d'ici ! 
    // Il est déjà géré par lvglTask dans lvlgdriver.cpp
    
    // On peut ralentir un peu cette tâche pour alléger le processeur (15ms c'était très agressif)
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