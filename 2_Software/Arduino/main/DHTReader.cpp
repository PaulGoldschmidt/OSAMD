#include "DHTReader.h"

#ifdef DHTTYPE
  #ifdef ESP32DEVKIT
  s72::DHTReader::DHTReader(byte const pin) : pin(pin), dht(new DHTesp()), readDelayMs(60000UL) {
  }

  s72::DHTReader::DHTReader(byte const pin, unsigned long const readDelayMs) : pin(pin), dht(new DHTesp()), readDelayMs(readDelayMs) {
  }
  #else
  s72::DHTReader::DHTReader(byte const pin) : pin(pin), dht(new DHT(pin, DHTTYPE)), readDelayMs(60000UL) {
  }

  s72::DHTReader::DHTReader(byte const pin, unsigned long const readDelayMs) : pin(pin), dht(new DHT(pin, DHTTYPE)), readDelayMs(readDelayMs) {
  }
  #endif
#else
  s72::DHTReader::DHTReader(byte const pin) : pin(pin), readDelayMs(60000UL) {
  }

  s72::DHTReader::DHTReader(byte const pin, unsigned long const readDelayMs) : pin(pin), readDelayMs(readDelayMs) {
  }
#endif




void s72::DHTReader::setup() {
// die Methode bleibt leer, wenn kein DHT verbaut ist
  #ifdef DHTTYPE
    #ifdef ESP32DEVKIT
      dht->setup(this->pin, DHTTYPE);
      Serial.print(F("DHTReader setup done (pin "));
      Serial.print(this->pin);
      Serial.println(F(")"));
      return;
    #else
      dht->begin();
    #endif
  #endif
  Serial.println(F("DHTReader setup done"));
}

#ifdef DHTTYPE
bool differs(float const val1, float const val2, float const epsilon) {
  bool const val1nan = isnan(val1);
  if (val1nan != isnan(val2)) return true;  // nur einer der beiden Werte ist nan, der andere nicht
  if (val1nan) return false;                // beide Werte sind nan
  float const diff = abs(val1 - val2);
  return diff >= epsilon;
}
#endif

void s72::DHTReader::update(){
// die Methode bleibt leer, wenn kein DHT verbaut ist
#ifdef DHTTYPE
  // DHTs mögen nicht zu häufig abgefragt werden
  unsigned long now = millis();
  if ((this->lastReadTime > 0) && ((now - this->lastReadTime) < this->readDelayMs)) return;
  
  this->lastReadTime = now;

  // Temperatur und Feuchtigkeit auslesen
#ifdef ESP32DEVKIT
  TempAndHumidity data = dht->getTempAndHumidity();
  float t = data.temperature;
  float h = data.humidity;
#else
  float h = dht->readHumidity();
  float t = dht->readTemperature(tempinfahrenheit);
#endif

  // hat sich mindestens einer der beiden Werte signifikant verändert?
  bool const significant_difference = differs(h, this->prev_humidity, MIN_HUMIDITY_DIFF_FOR_EVENT) || differs(t, this->prev_temperature, MIN_TEMPERATURE_DIFF_FOR_EVENT);
  if (significant_difference) {
    s72::DHTEvent::raise(this->pin, t, h);
    this->prev_humidity = h;
    this->prev_temperature = t;
  }
#endif
}



s72::DHTListener *s72::DHTEvent::listener[2];

void s72::DHTEvent::registerListener(DHTListener &new_listener) {
// die Methode bleibt leer, wenn kein DHT verbaut ist
#ifdef DHTTYPE
  if (listener[0] == nullptr) {
    listener[0] = &new_listener;
    Serial.println(F("DHTEvent: listener 1 registered"));
  } else if (listener[1] == nullptr) {
    listener[1] = &new_listener;
    Serial.println(F("DHTEvent: listener 2 registered"));
  } else {
    Serial.println(F("DHTEvent: ERROR! listener 3 not registered"));
  }
#endif
}

void s72::DHTEvent::raise(byte const pin, float const temperature, float const humidity) {
// die Methode bleibt leer, wenn kein DHT verbaut ist
#ifdef DHTTYPE
  Serial.println(F("DHTEvent: raise"));
  if (listener[0] == nullptr) return;
  Serial.println(F("  send to listeners"));
  DHTEvent event(pin, temperature, humidity);
  listener[0]->onEvent(event);
  if (listener[1] != nullptr) listener[1]->onEvent(event);
#endif
}