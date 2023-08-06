#include "MQ135Reader.h"

s72::MQ135Reader::MQ135Reader(byte const pin) :
  pin(pin),
  rzero(76.63),
  rload(10.0),
  readDelayMs(s72::MQ135Reader::READ_DELAY_MS),
  preheatDelayMs(s72::MQ135Reader::PREHEAT_DURATION_MS)
{
  startup_time = millis();
}

s72::MQ135Reader::MQ135Reader(byte const pin, unsigned long const readDelayMs, unsigned long const preheatDelayMs) :
  pin(pin),
  rzero(76.63),
  rload(10.0),
  readDelayMs(readDelayMs),
  preheatDelayMs(preheatDelayMs)
{
  startup_time = millis();
}

s72::MQ135Reader::MQ135Reader(byte const pin, float const rzero, float const rload) :
  pin(pin),
  rzero(rzero),
  rload(rload),
  readDelayMs(s72::MQ135Reader::READ_DELAY_MS),
  preheatDelayMs(s72::MQ135Reader::PREHEAT_DURATION_MS)
{
  startup_time = millis();
}

s72::MQ135Reader::MQ135Reader(byte const pin, float const rzero, float const rload, unsigned long const readDelayMs, unsigned long const preheatDelayMs) :
  pin(pin),
  rzero(rzero),
  rload(rload),
  readDelayMs(readDelayMs),
  preheatDelayMs(preheatDelayMs)
{
  startup_time = millis();
}

bool s72::MQ135Reader::is_preheating() const {
  return (millis() - this->startup_time) <= s72::MQ135Reader::PREHEAT_DURATION_MS;
}

void s72::MQ135Reader::setup() {
  Serial.println(F("MQ135Reader setup done"));
}

void s72::MQ135Reader::update() {
  // Sensor ignorieren, solange er nicht ausreichend aufgeheizt ist (kann mit ignorePreheat() übersteuert werden)
  if (is_preheating() && !this->ignore_preheat) return;

  // zeitlichen Mindestabstand zwischen zwei Messungen einhalten
  unsigned long const now = millis();
  if ((now - this->lastReadTime) < this->readDelayMs) return;

  // Widerstand des Sensors auslesen
  unsigned int const val = analogRead(this->pin);
#ifdef ESP32DEVKIT
  // Auflösung des Analogports am ESP: 12 bit (0..4095)
  float const resistance = ((4095.0 / (float)val) - 1.0) * this->rload;
#else
  // Auflösung des Analogports am Arduino: 10 bit (0..1023)
  float const resistance = ((1023.0 / (float)val) - 1.0) * this->rload;
#endif

  // Physik- und Mathe-Voodoo anwenden, um ppm bzw. korrigierte ppm auszurechnen (siehe MQ135-lib: https://github.com/Phoenix1747/MQ135)
  float const rzero = resistance * VOODOO;
  float const ppm = PARA * pow((resistance / this->rzero), -PARB);;

  // Werte korrigieren, falls Temperatur und Luftfeuchtigkeit bekannt sind
  float correctedRZero;
  float correctedPPM;
  int diff;
  if (this->humidity >= 0.0) {
    float const correctionFactor = (this->temperature < 20) ? (CORA * this->temperature * this->temperature - CORB * this->temperature + CORC - (this->humidity - 33.0) * CORD) : (CORE * this->temperature + CORF * this->humidity + CORG);
    float const correctedResistance = resistance / correctionFactor;
    correctedRZero = correctedResistance * VOODOO;
    correctedPPM = PARA * pow((correctedResistance / this->rzero), -PARB);
    diff = abs(correctedPPM - this->prev_correctedPPM);
  } else {
    correctedRZero = rzero;
    correctedPPM = ppm;
    diff = abs(ppm - this->prev_ppm);
  }

  // MQ135-Listener informieren, falls sich der PPM-Wert signifikant verändert hat
  if (diff >= s72::MQ135Reader::MIN_PPM_DIFF_FOR_EVENT) {
    MQ135Event::raise(
      this->pin,
      rzero,
      correctedRZero,
      resistance,
      ppm,
      correctedPPM,
      is_preheating()
    );
    this->prev_correctedPPM = correctedPPM;
    this->prev_ppm = ppm;
  }

  this->lastReadTime = millis();
}

void s72::MQ135Reader::calibrate(float const temperature, float const humidity) {
  this->temperature = temperature;
  this->humidity = humidity;
}

void s72::MQ135Reader::ignorePreheat(bool const ignore) {
  this->ignore_preheat = ignore;
}

void s72::MQ135Reader::ignorePreheat() {
  ignorePreheat(true);
}


void s72::MQ135Reader::onEvent(DHTEvent &event) {
  float t = event.temperature;
  if (!isnan(t)) {
    this->temperature = t;
  }
  float h = event.humidity;
  if (!isnan(h)) {
    this->humidity = h;
  }
#ifdef DEBUG
  Serial.println(F("MQ135Reader: DHTEvent"));
  Serial.print("  T: ");
  Serial.print(t);
  Serial.print(" H: ");
  Serial.println(h);
#endif
}






s72::MQ135Listener *s72::MQ135Event::listener;

void s72::MQ135Event::registerListener(s72::MQ135Listener &new_listener) {
  listener = &new_listener;
}

void s72::MQ135Event::raise(byte const pin, float const rzero, float const correctedRZero, float const resistance, float const ppm, float const correctedPPM, bool const preheating) {
  if (listener == nullptr) return;
  s72::MQ135Event event(pin, rzero, correctedRZero, resistance, ppm, correctedPPM, preheating);
  listener->onEvent(event);
}
