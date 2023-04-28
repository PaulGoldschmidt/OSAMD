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

//String const OSAMD_VERSION = "V. 1.2A";
char const OSAMD_VERSION[] = "V. 1.2A";

enum class DisplayState {
  ShowStartScreen,
  ShowPreheat,
  InitShowReadings,
  ShowReadings,
  ShowConfigBuzzer,
  ShowConfigBacklight
};


class Display : public ButtonReleaseListener, public s72::DHTListener, public s72::MQ135Listener {
private:
  
  static unsigned long const START_SCREEN_DURATION_MS = 2500;      // Startnachricht 2,5 Sekunden anzeigen
  static unsigned long const CONFIG_TIMEOUT_MS = 30000;            // Wenn das Konfigurations 30 Sekunden nicht genutzt wird, wieder in Anzeigemodus wechseln
  
  LiquidCrystal_I2C lcd;
  byte const width;
  byte const height;

  DisplayState current_state = DisplayState::ShowStartScreen;
  DisplayState prev_state = DisplayState::ShowStartScreen;
  unsigned long current_state_start_time = 0;
  bool update_buzzer_option = false;
  bool update_backlight_option = false;

  float cached_temp = 0.0;
  float cached_hum = -1.0;
  float cached_ppm = -1.0;

  s72::Config *config;
  s72::LED *led;
  s72::Buzzer *buzzer;
  s72::MQ135Reader *mq135reader;

  void displayHumTemp(float const temp, float const hum);
  void displayPpm(float const ppm);
  void switchToState(DisplayState new_state);
  void switchToState(DisplayState new_state, unsigned long time);
  Display() = default;

public:

  static unsigned char const GLYPH_DEGREE = 223;  // Nummer des Grad-Zeichens (°)


  // ctor, dtor, operations
  Display(byte const address, byte const width, byte const height);
  Display(const Display &) = delete; // no copying
  Display &operator=(const Display &) = delete;

  void draw_progress_bar(unsigned char percent_value);
  void draw_progress_bar(unsigned char percent_value, unsigned char row);
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
