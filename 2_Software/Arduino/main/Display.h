#ifndef OSAMD_DISPLAY_H_
#define OSAMD_DISPLAY_H_

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>  // LiquidCrystal I2C - Frank de Brabander - Version 1.1.2
#include "Button.h"
#include "DHTReader.h"
#include "MQ135Reader.h"
#include "Buzzer.h"
#include "Led.h"


namespace s72 {

  char const OSAMD_VERSION[] = "V. 1.3";

  // Mögliche Zustände des Displays (-> was grade auf dem 16x2-Display dargestellt werden soll)
  enum class DisplayState {
    ShowStartScreen,      // einmal beim Start die Version anzeigen
    ShowPreheat,          // 15 Minuten Vorheizen (mit Fortschrittsbalken) anzeigen
    InitShowReadings,     // Vorbereiten der Anzeige der Sensordaten
    ShowReadings,         // Anzeige der Sensordaten
    ShowConfigBuzzer,     // Menü: Optionen für den Buzzer (an/aus)
    ShowConfigBacklight,  // Menü: Optionen für die Beleuchtung des Displays (an/aus)
    ShowConfigLED         // Menü: Optionen für die Status-LED (an/aus)
  };


  // Ansteuerung des 16x2-Display und Verwaltung der Menüs
  // Das Display muss auf Tastendrücke reagieren -> ButtonReleaseListener,
  //  die Temperatur und Luftfeuchtigkeit anzeigen -> DHTListener und
  //  die MQ135-Sensordaten (als PPM) darstellen -> MQ135Listener
  class Display : public ButtonReleaseListener, public s72::DHTListener, public s72::MQ135Listener {
  private:
    
    static byte const GLYPH_FULL = 0;
    static byte const GLYPH_ONE_FIFTH = 1;
    static byte const GLYPH_TWO_FIFTH = 2;
    static byte const GLYPH_THREE_FIFTH = 3;
    static byte const GLYPH_FOUR_FIFTH = 4;
    static byte const GLYPH_DEGREE = 223;  // Nummer des Grad-Zeichens (°)

    static byte const ONE_FIFTH = 0x10;
    static byte const TWO_FIFTH = 0x18;
    static byte const THREE_FIFTH = 0x1C;
    static byte const FOUR_FIFTH = 0x1E;
    static byte const FULL = 0x1F;

    static byte const CHAR_WIDTH_IN_PIXEL = 5;

    static unsigned long const START_SCREEN_DURATION_MS = 2500;      // Startnachricht 2,5 Sekunden anzeigen
    static unsigned long const CONFIG_TIMEOUT_MS = 30000;            // Wenn das Konfigurationsmenü 30 Sekunden nicht genutzt wird, wieder in Anzeigemodus wechseln
    
    // 16x2 Display (Hardware)
    LiquidCrystal_I2C lcd;
    byte const width;
    byte const height;

    // Zustand des Displays
    DisplayState current_state = DisplayState::ShowStartScreen;
    DisplayState prev_state = DisplayState::ShowStartScreen;
    unsigned long current_state_start_time = 0;                       // wie lange schon im aktuellen Zustand (für Menü-Timeout)
    bool update_buzzer_option = false;                                // hat sich die Konfiguration des Buzzers geändert?
    bool update_backlight_option = false;                             // hat sich die Konfiguration der Beleuchtung geändert?
    bool update_led_option = false;                                   // hat sich die Konfiguration der LED geändert?

    // Anzuzeigende Daten (werden hierin zwischengespeichert, wenn das Display im Menü ist und daher die Messwerte nicht anzeigen kann)
    float cached_temp = 0.0;
    float cached_hum = -1.0;
    float cached_ppm = -1.0;

    s72::Config *config;
    s72::LED *led;
    s72::Buzzer *buzzer;
    s72::MQ135Reader *mq135reader;

    void create_glyph(unsigned char const number, unsigned char const value);
    void displayHumTemp(float const temp, float const hum);
    void displayPpm(float const ppm);
    void switchToState(DisplayState new_state);
    void switchToState(DisplayState new_state, unsigned long time);
    Display() = default;

  public:

    Display(byte const address, byte const width, byte const height);

    Display(const Display &) = delete; // no copying
    Display &operator=(const Display &) = delete;

    void draw_progress_bar(byte const percent_value);
    void draw_progress_bar(byte percent_value, byte const row);
    void setup(s72::Config &config, s72::LED &led, s72::Buzzer &buzzer, s72::MQ135Reader &mq135reader);
    void update();

    // ButtonReleaseListener
    virtual void onEvent(s72::ButtonReleaseEvent &event);

    // DHTListener
    virtual void onEvent(s72::DHTEvent &event);

    // MQ135Listener
    virtual void onEvent(s72::MQ135Event &event);

  };

}
#endif
