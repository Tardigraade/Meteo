#include "lvgl.h"

static float t_interieur = 0.0;
static float h_interieur = 0.0;
static float t_exterieur = 0.0;
static float h_exterieur = 0.0;
static bool nouvelles_donnees_dispo = false;

static lv_obj_t * label_int;
static lv_obj_t * label_ext; // Utilisé correctement de manière globale maintenant

void testLvgl(void)
{
    static lv_style_t style_carre;
    lv_style_init(&style_carre);
    lv_style_set_bg_opa(&style_carre, LV_OPA_COVER);
    lv_style_set_border_color(&style_carre, lv_color_hex(0x000000)); // Bordure noire
    lv_style_set_border_width(&style_carre, 4); 
    lv_style_set_radius(&style_carre, 10); 

    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(screen, 8, 0);        
    lv_obj_set_style_pad_column(screen, 8, 0);     

    // --- CARD DASHBOARD ---
    lv_obj_t * card_dashboard = lv_obj_create(screen);
    lv_obj_set_size(card_dashboard, 224, LV_PCT(100)); 
    lv_obj_add_style(card_dashboard, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_dashboard, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_dashboard, lv_palette_main(LV_PALETTE_GREEN), 0);

    lv_obj_t * label_dash = lv_label_create(card_dashboard);
    lv_obj_center(label_dash);
    lv_label_set_text(label_dash, "Dashboard");

    // --- COLONNE DROITE ---
    lv_obj_t * colonne_droite = lv_obj_create(screen);
    lv_obj_set_size(colonne_droite, 224, LV_PCT(100));
    lv_obj_set_style_bg_opa(colonne_droite, LV_OPA_TRANSP, 0); 
    lv_obj_set_style_border_width(colonne_droite, 0, 0);      
    lv_obj_set_style_pad_all(colonne_droite, 0, 0);
    lv_obj_set_flex_flow(colonne_droite, LV_FLEX_FLOW_COLUMN); 
    lv_obj_set_style_pad_row(colonne_droite, 8, 0);            

    // --- CARD INTERIEUR ---
    lv_obj_t * card_interieur = lv_obj_create(colonne_droite);
    lv_obj_set_size(card_interieur, LV_PCT(100), 0); 
    lv_obj_set_flex_grow(card_interieur, 1);
    lv_obj_add_style(card_interieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_interieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_interieur, lv_palette_main(LV_PALETTE_BLUE), 0);

    label_int = lv_label_create(card_interieur);
    lv_obj_center(label_int);
    // Correction : Utilisation de _fmt pour l'initialisation avec les floats
    lv_label_set_text_fmt(label_int, "Interieur :\nTemp: %.1f°C\nHum: %.1f%%", t_interieur, h_interieur); 
 
    // --- CARD EXTERIEUR ---
    lv_obj_t * card_exterieur = lv_obj_create(colonne_droite);
    lv_obj_set_size(card_exterieur, LV_PCT(100), 0); 
    lv_obj_set_flex_grow(card_exterieur, 1);
    lv_obj_add_style(card_exterieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_exterieur, LV_SCROLLBAR_MODE_OFF);

    // Correction : Affectation à la variable GLOBALE (pas de "lv_obj_t *")
    label_ext = lv_label_create(card_exterieur);
    lv_obj_center(label_ext);
    // Correction : Utilisation de _fmt pour l'initialisation
    lv_label_set_text_fmt(label_ext, "Exterieur :\nTemp: %.1f°C\nHum: %.1f%%", t_exterieur, h_exterieur); 
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
  Serial.println("Setup done");
}

void loop()
{
    unsigned long currentMillis = millis();
    
    if (currentMillis - lastDHTRead >= dhtInterval) {
        lastDHTRead = currentMillis;
        
        float h = dht.readHumidity();
        float t = dht.readTemperature();
        float h2 = dht2.readHumidity();
        float t2 = dht2.readTemperature();

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
    
    delay(10); 
}

void myTask(void *pvParameters)
{
  TickType_t xLastWakeTime = xTaskGetTickCount();
  while (1)
  {
    if (nouvelles_donnees_dispo) {
        // Correction : On met à jour l'intérieur ET l'extérieur sur leurs labels respectifs
        if (label_int != NULL) {
            lv_label_set_text_fmt(label_int, "Interieur :\nTemp: %.1f°C\nHum: %.1f%%", t_interieur, h_interieur);
        }
        if (label_ext != NULL) {
            lv_label_set_text_fmt(label_ext, "Exterieur :\nTemp: %.1f°C\nHum: %.1f%%", t_exterieur, h_exterieur);
        }
        nouvelles_donnees_dispo = false; 
    }
    
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