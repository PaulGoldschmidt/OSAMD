#include "Config.h"
#include "Display.h"
#include "Buzzer.h"
#include "DHTReader.h"
#include "MQ135Reader.h"
#include "Led.h"
#include "Button.h"

s72::Config config;
s72::Display display(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);
s72::Button button(s72::BUTTON_PIN, s72::BUTTON_PIN_IS_ANALOG, s72::DEBOUNCE_DELAY_MS, s72::HIGH_MEANS_PRESSED);
s72::Buzzer buzzer(s72::BUZZER_PIN);
s72::LED led(s72::LED_RED_PIN, s72::LED_GREEN_PIN, s72::LED_BLUE_PIN);
s72::DHTReader dhtReader(s72::DHT_PIN);
s72::MQ135Reader mq135Reader(s72::MQ135_PIN);

void setup() {

  // Serielle Kommunikation zum PC über 115200 baud/sekunde:
  Serial.begin(115200);

  config.setup();
  button.setup();
  dhtReader.setup();
  buzzer.setup();
  led.setup();
  mq135Reader.setup();
  display.setup(config, led, buzzer, mq135Reader); 

  #ifdef WEMOS_D1_MINI_LIGHT
    //WLAN initialisieren
  #endif

  //TODO: Aktive Kalibrierung des Sensors
}



void loop() {
  dhtReader.update();   // vor mq135 und display, da dht-Events von mq135 und display verwendet werden
  mq135Reader.update(); // vor display, da mq135-Events von display verwendet werden
  button.update();      // vor display, da button-Events von display verwendet werden
  display.update();     // vor buzzer und led, da display buzzer und led ansteuert
  buzzer.update();
  led.update();
}

