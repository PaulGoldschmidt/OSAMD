#ifndef OSAMD_CONFIG_H_
#define OSAMD_CONFIG_H_

#include <Arduino.h>


namespace s72 {

/*
 * ----------------------
 * Start Codekonfiguration
 * ======================
 */

// Einstellungen für den Betrieb des MQ 135-Sensors
//#define BAD_AIR_VALUE 768 //Die maximal erlaubte Schlecht-Luftqualität (Standard: 512, Maximal: 1024, Grundwert des Sensors bei sehr guter Luft: circa 250)
//#define PERCENT_GOOD 65
//#define PERCENT_OK 75

unsigned long const LONG_KEYPRESS_MS = 1000; // Tastendruck wird als langer Tastendruck gewertet, wenn die Taste mindestens diese Dauer in Millisekunden gedrückt wird

/*
 * ======================
 * Ende Codekonfiguration
 * ----------------------
*/

// aktivieren, um diverse Infos über Serial zu bekommen
#define DEBUG

/*
 * ----------------------
 * Start Hardwarekonfiguration
 * ======================
 */

// Für welchen Mikrocontroller soll der Code kompiliert werden (Arduino Nano, WEMOS D1 Mini light oder (ESP32)? Es darf nur ein Board aktiviert sein!
//#define ARDUINONANO
#define WEMOS_D1_MINI_LIGHT
//#define ESP32DEVKIT

// Ist auf der Platine ein Temperatur- und Luftfeuchtesensor verbaut?
//#define TEMPINSTALLED // auskommentieren, wenn kein DHT-Temperatursensor verwendet wird.
// Welcher Sensor ist verwendetet: DHT11, DHT22 (auch (AM2302), AM2321) und DHT21? Nur relevant, wenn TEMPINSTALLED definiert ist
// WICHTIG: für ESP32 muss ein DHTesp:: vorangestellt werden
#define DHTTYPE DHT22
//#define DHTTYPE DHTesp::DHT22


// Konfiguration des Display
#define LCD_I2C_ADDRESS 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2


#ifdef WEMOS_D1_MINI_LIGHT
  // D1 ist SCL für I2C LCD
  // D2 ist SDA für I2C LCD

  const byte BUZZER_PIN = D8; // darf NICHT an D4, sonst lässt sich der WEMOS nur noch außerhalb der Schaltung / Platine flashen

  const byte LED_RED_PIN = D5;
  const byte LED_BLUE_PIN = D6;
  const byte LED_GREEN_PIN = D3;

  const byte MQ135_PIN = A0;

  const byte BUTTON_PIN = D4; // ist auch gleichzeitig die interne LED
  const bool BUTTON_PIN_IS_ANALOG = false;
  const unsigned int DEBOUNCE_DELAY_MS = 50;
  const boolean HIGH_MEANS_PRESSED = false;

  const byte DHT_PIN = D7;
#endif

#ifdef ESP32DEVKIT

  // Work in progress!! Aktuell nur für Wokwi-Simulation -> https://wokwi.com/projects/363144454759988225
  
  const byte BUZZER_PIN = 19; // ESP32 haben keine Pin-Aliase -> D19 = GPIO19 = 19

  const byte LED_RED_PIN = 27;
  const byte LED_BLUE_PIN = 25;
  const byte LED_GREEN_PIN = 26;

  const byte MQ135_PIN = 34;

  const byte BUTTON_PIN = 4;
  const bool BUTTON_PIN_IS_ANALOG = false;
  const unsigned int DEBOUNCE_DELAY_MS = 50;
  const boolean HIGH_MEANS_PRESSED = false;

  const byte DHT_PIN = 18;

#endif


#ifdef ARDUINONANO
  const byte BUZZER_PIN = 6;

  const byte LED_RED_PIN = 9;
  const byte LED_BLUE_PIN = 10;
  const byte LED_GREEN_PIN = 11;

  const byte MQ135_PIN = A0;

  const byte BUTTON_PIN = A6;
  const bool BUTTON_PIN_IS_ANALOG = true;
  const unsigned int DEBOUNCE_DELAY_MS = 50;
  const boolean HIGH_MEANS_PRESSED = false;

  const byte DHT_PIN = 2;
#endif

  const bool tempinfahrenheit = false;
  const float MIN_TEMPERATURE_DIFF_FOR_EVENT = 0.1;
  const float MIN_HUMIDITY_DIFF_FOR_EVENT = 0.1;

/*
 * ======================
 * Ende Hardwarekonfiguration
 * ----------------------
*/

#if defined(ESP32DEVKIT) || defined(WEMOS_D1_MINI_LIGHT)
  #define ESPEEPROM
#endif


class Config
{
private:
  bool buzzer_enabled = true;
  bool backlight_enabled = true;

public:
  static const unsigned char TRUE_VALUE = 1;
  static const unsigned char FALSE_VALUE = 0;
  static const unsigned int EEPROM_BUZZER = 2;
  static const unsigned int EEPROM_BACKLIGHT = 1;
  static const unsigned int EEPROM_INIT_DONE = 0;
  static const char FLAG_INIT_DONE = 'O';

  bool const is_buzzer_enabled();
  bool const is_backlight_enabled();
  void toggle_buzzer_enabled();
  void toggle_backlight_enabled();
  void set_buzzer_enabled(bool const state);
  void set_backlight_enabled(bool const state);
  void const save();
  void setup();
};

}

#endif