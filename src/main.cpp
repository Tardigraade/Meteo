#include "lvgl.h"




void testLvgl(void)
{
    /* 1. Création d'un style super simple pour les carrés (Fond blanc, bordure noire) */
    static lv_style_t style_carre;
    lv_style_init(&style_carre);
    //lv_style_set_bg_color(&style_carre, lv_color_hex(0xffffff)); // Fond blanc
    lv_style_set_bg_opa(&style_carre, LV_OPA_COVER);
    lv_style_set_border_color(&style_carre, lv_color_hex(0x000000)); // Bordure noire
    lv_style_set_border_width(&style_carre, 4); // Épaisseur de la bordure
    lv_style_set_radius(&style_carre, 10); // Coins légèrement arrondis comme le dessin

    /* 2. Configurer l'écran pour aligner la partie Gauche et la partie Droite côte à côte */
    lv_obj_t * screen = lv_screen_active();
    lv_obj_set_flex_flow(screen, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(screen, 8, 0);        // Marge au bord de l'écran
    lv_obj_set_style_pad_column(screen, 8, 0);     // Espace entre le bloc gauche et droit
    lv_obj_t * card_dashboard = lv_obj_create(screen);
    lv_obj_set_size(card_dashboard, 224, LV_PCT(100)); // Largeur: 224px, Hauteur: toute la hauteur
    lv_obj_add_style(card_dashboard, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_dashboard, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t * label_dash = lv_label_create(card_dashboard);
    lv_obj_center(label_dash);
    lv_label_set_text(label_dash, "Dashboard");
    lv_obj_set_style_bg_color(card_dashboard, lv_palette_main(LV_PALETTE_GREEN), 0);// couleur
    lv_obj_t * colonne_droite = lv_obj_create(screen);
    lv_obj_set_size(colonne_droite, 224, LV_PCT(100));
    lv_obj_set_style_bg_opa(colonne_droite, LV_OPA_TRANSP, 0); // Invisible
    lv_obj_set_style_border_width(colonne_droite, 0, 0);      // Sans bordure
    lv_obj_set_style_pad_all(colonne_droite, 0, 0);
    
    lv_obj_set_flex_flow(colonne_droite, LV_FLEX_FLOW_COLUMN); // Empilement vertical
    lv_obj_set_style_pad_row(colonne_droite, 8, 0);            // Espace entre les deux carrés
 
    lv_obj_t * card_interieur = lv_obj_create(colonne_droite);
    lv_obj_set_size(card_interieur, LV_PCT(100), 0); // Largeur 100%, hauteur gérée par le flex grow
    lv_obj_set_flex_grow(card_interieur, 1);
    lv_obj_add_style(card_interieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_interieur, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(card_interieur, lv_palette_main(LV_PALETTE_BLUE), 0);// couleur
    lv_obj_t * label_int = lv_label_create(card_interieur);
    lv_obj_center(label_int);
    lv_label_set_text(label_int, "Interieur");
    lv_label_set_text(label_int, " Interieur : \n Temp: 22°C\nHum: 45%"); // Exemple de données
 
    lv_obj_t * card_exterieur = lv_obj_create(colonne_droite);
    lv_obj_set_size(card_exterieur, LV_PCT(100), 0); // Largeur 100%, hauteur gérée par le flex grow
    lv_obj_set_flex_grow(card_exterieur, 1);
    lv_obj_add_style(card_exterieur, &style_carre, 0);
    lv_obj_set_scrollbar_mode(card_exterieur, LV_SCROLLBAR_MODE_OFF);
    //lv_obj_set_style_bg_color(card_interieur, lv_palette_main(WHITE), 0);// couleur
    lv_obj_t * label_ext = lv_label_create(card_exterieur);
    lv_obj_center(label_ext);
    lv_label_set_text(label_ext, "Exterieur");
    lv_label_set_text(label_ext, "Exterieur : \nTemp: 18°C\nHum: 60%"); // Exemple de données
}

#ifdef ARDUINO

#include "lvglDrivers.h"

void mySetup()
{
  testLvgl();
  Serial.begin(115200);
}

void loop()
{
    Serial.println("Looping...");
    delay(1000);

    
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

