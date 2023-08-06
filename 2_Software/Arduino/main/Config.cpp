#include "Config.h"
#include <EEPROM.h>


bool eeprom_bool_value(const unsigned int position) {
  return EEPROM.read(position) == s72::Config::TRUE_VALUE;
}

unsigned char bool_to_value(bool const state) {
  return state ? s72::Config::TRUE_VALUE : s72::Config::FALSE_VALUE;
}


// einheitliches schreibsparsames Aktualisieren des Flash für alle MCU
// ESP32 kennt kein EEPROM.update, das macht dessen EEPROM.write gleich mit. Alle anderen schreiben aber stumpf auch identische Werte...
void update_eeprom(const unsigned int position, unsigned char value) {
  Serial.print(F("Config update_eeprom pos "));
  Serial.print(position);
  Serial.print(F(" val "));
  Serial.println(value);
  unsigned char const current = EEPROM.read(position);
  if (current == value) {
    Serial.println(F("  no change"));
    return;
  }
    Serial.println(F("  write change to EEPROM"));
  EEPROM.write(position, value);
  // verify
  if (EEPROM.read(position) != value) {
    Serial.print(F("ERROR: write to EEPROM failed at position "));
    Serial.println(position);
  }
}

bool s72::Config::is_buzzer_enabled() const {
  return this->buzzer_enabled;
}

bool s72::Config::is_backlight_enabled() const {
  return this->backlight_enabled;
}

bool s72::Config::is_led_enabled() const {
  return this->led_enabled;
}

void s72::Config::set_buzzer_enabled(bool const state) {
  Serial.print(F("Config set_buzzer_enabled "));
  Serial.println(state);
  this->buzzer_enabled = state;
}

void s72::Config::set_backlight_enabled(bool const state) {
  Serial.print(F("Config set_backlight_enabled "));
  Serial.println(state);
  this->backlight_enabled = state;
}

void s72::Config::set_led_enabled(bool const state) {
  Serial.print(F("Config set_led_enabled "));
  Serial.println(state);
  this->led_enabled = state;
}

void s72::Config::toggle_buzzer_enabled() {
  Serial.print(F("Config toggle_buzzer_enabled "));
  this->buzzer_enabled = !this->buzzer_enabled;
}

void s72::Config::toggle_backlight_enabled() {
  Serial.print(F("Config toggle_backlight_enabled "));
  this->backlight_enabled = !this->backlight_enabled;
}

void s72::Config::toggle_led_enabled() {
  Serial.print(F("Config toggle_led_enabled "));
  this->led_enabled = !this->led_enabled;
}



void s72::Config::save() const {
  Serial.println(F("Config save"));
  update_eeprom(s72::Config::EEPROM_BUZZER, bool_to_value(this->buzzer_enabled));
  update_eeprom(s72::Config::EEPROM_BACKLIGHT, bool_to_value(this->backlight_enabled));
  update_eeprom(s72::Config::EEPROM_LED, bool_to_value(this->led_enabled));

#ifdef ESPEEPROM
  EEPROM.commit(); // ESP8266/ESP32: EEPROM.write schreibt nur in einen Puffer. Aus dem Puffer jetzt wirklich ins Flash schreiben.
#endif

}

void s72::Config::setup() {

#ifdef ESPEEPROM
  EEPROM.begin(s72::Config::EEPROM_SIZE); // ESP8266/ESP32: vor Zugriff auf EEPROM muss es mit der gewünschten Größe initialisiert werden.
#endif

  if (EEPROM.read(s72::Config::EEPROM_INIT_DONE) != s72::Config::FLAG_INIT_DONE) {
    Serial.println(F("Config setup: initialize EEPROM"));
    // beim ersten Start des Mikrocontrollers enthält das EEPROM noch keine Daten
    // -> mit sinnvollen Daten vorbelegen
    update_eeprom(s72::Config::EEPROM_INIT_DONE, s72::Config::FLAG_INIT_DONE);
    update_eeprom(s72::Config::EEPROM_BACKLIGHT, s72::Config::TRUE_VALUE);
    update_eeprom(s72::Config::EEPROM_BUZZER, s72::Config::TRUE_VALUE);
    update_eeprom(s72::Config::EEPROM_LED, s72::Config::TRUE_VALUE);
  }
  // Konfigurationswerte aus dem EEPROM holen
  this->buzzer_enabled = eeprom_bool_value(s72::Config::EEPROM_BUZZER);
  this->backlight_enabled = eeprom_bool_value(s72::Config::EEPROM_BACKLIGHT);
  this->led_enabled = eeprom_bool_value(s72::Config::EEPROM_LED);

  Serial.print(F("Config setup done (buzzer: "));
  Serial.print(this->buzzer_enabled);
  Serial.print(F(" backlight: "));
  Serial.print(this->backlight_enabled);
  Serial.print(F(" LED: "));
  Serial.print(this->led_enabled);
  Serial.println(F(")"));
}