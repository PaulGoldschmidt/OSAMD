#include "Button.h"

s72::Button::Button(byte const pin, bool const is_analog_pin) : pin(pin), is_analog_pin(is_analog_pin), debounceDelay(s72::Button::DEBOUNCE_DEFAULT_MS), high_means_pressed(true)
{
  init();
}

s72::Button::Button(byte const pin, bool const is_analog_pin, unsigned int const debounceDelay) : pin(pin), is_analog_pin(is_analog_pin), debounceDelay(debounceDelay), high_means_pressed(true)
{
  init();
}

s72::Button::Button(byte const pin, bool const is_analog_pin, bool const high_means_pressed) : pin(pin), is_analog_pin(is_analog_pin), debounceDelay(DEBOUNCE_DEFAULT_MS), high_means_pressed(high_means_pressed)
{
  init();
}

s72::Button::Button(byte const pin, bool const is_analog_pin, unsigned int const debounceDelay, bool const high_means_pressed) : pin(pin), is_analog_pin(is_analog_pin), debounceDelay(debounceDelay), high_means_pressed(high_means_pressed)
{
  init();
}


void s72::Button::init()
{
  pinMode(this->pin, INPUT);
  this->currentState = high_means_pressed ? LOW : HIGH; // initialize with "not pressed"
  unsigned long now = millis();
  this->currentStateStartTime = now;
  this->lastStateChangeTime = now;
}


unsigned long const s72::Button::stateDuration()
{
  return millis() - this->lastStateChangeTime;
}

void s72::Button::setup()
{
  Serial.print(F("Button setup done (pin "));
  Serial.print(this->pin);
  Serial.print(F(", debounce delay "));
  Serial.print(this->debounceDelay);
  Serial.println(F(")"));
}


void s72::Button::update()
{
  unsigned long now = millis(); // get the current time stamp only once

  // Tasterstatus holen
  int current_read;
  if (this->is_analog_pin) {
    current_read = analogRead(this->pin) < 512;
  } else {
    current_read = digitalRead(this->pin);
  }
  // wenn Änderung: Zeitpunkt der Änderung merken
  if (this->currentState != current_read) {
    this->lastStateChangeTime = now;
    this->currentState = current_read;
  }
  
  // Änderung erst als stabil ansehen, wenn der Taster lange genug den selben Status meldet (=> entprellen)
  unsigned long duration_since_last_change = now - this->lastStateChangeTime;
  if (duration_since_last_change <= this->debounceDelay) return;

  // hat sich der entprellte Zustand des Tasters geändert?
  if (current_read == this->currentStableState) return;

  // Dauer des vorheigen Zustands ausrechnen (this->currentStateStartTime enthält noch den vorherigen Zustand)
  unsigned long duration_last_state = now - this->currentStateStartTime;
  // Änderung des Zustands per Event melden (this->is_pressed enthält noch den vorherigen Zustand)
  if (this->is_pressed) {
    // von "gedrückt" zu "nicht gedrückt"
    s72::ButtonReleaseEvent::raise(this->pin, duration_last_state);
  } else {
    // von "nicht gedrückt" zu "gedrückt", nicht implementiert
    //ButtonPressedEvent::raise(this->pin, duration_last_state);
  }

  // Jetzigen Zustand merken
  this->currentStableState = current_read;
  this->currentStateStartTime = now;
  this->is_pressed = this->currentStableState == (this->high_means_pressed ? HIGH : LOW);
  
}




s72::ButtonReleaseListener *s72::ButtonReleaseEvent::listener;

void s72::ButtonReleaseEvent::registerListener(s72::ButtonReleaseListener &new_listener) {
  listener = &new_listener;
}



void s72::ButtonReleaseEvent::raise(byte const pin, unsigned long const pressed_duration) {
  if (listener == nullptr) return;
  Serial.println(F("ButtonReleaseEvent raised"));
  s72::ButtonReleaseEvent event(pin, pressed_duration);
  listener->onEvent(event);
}
