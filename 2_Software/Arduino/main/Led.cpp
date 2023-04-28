#include "Led.h"
#include "Config.h"


void printStateName(s72::LEDState const state) {
  switch (state)
  {
    case s72::LEDState::turn_on:
      Serial.print(F("turn_on"));
      break;
    case s72::LEDState::on:
      Serial.print(F("on"));
      break;
    case s72::LEDState::turn_off:
      Serial.print(F("turn_off"));
      break;
    case s72::LEDState::off:
      Serial.print(F("off"));
      break;
    case s72::LEDState::start_blinking_on:
      Serial.print(F("start_blinking_on"));
      break;
    case s72::LEDState::blinking_on:
      Serial.print(F("blinking_on"));
      break;
    case s72::LEDState::start_blinking_off:
      Serial.print(F("start_blinking_off"));
      break;
    case s72::LEDState::blinking_off:
      Serial.print(F("blinking_off"));
      break;
  }
}




s72::LED::LED(byte const pinRed, byte const pinGreen, byte const pinBlue) : pinRed(pinRed), pinGreen(pinGreen), pinBlue(pinBlue)
{}

void s72::LED::set_on(){
  switchToState(s72::LEDState::turn_on);
}

void s72::LED::set_off(){
  switchToState(s72::LEDState::turn_off);
}

void s72::LED::set_blinking(){
  this->infinite_blink = true;
  switchToState(s72::LEDState::start_blinking_on);
}

void s72::LED::set_blinking(unsigned int repetitions) {
  this->infinite_blink = false;
  this->repetitions = repetitions;
  switchToState(s72::LEDState::start_blinking_on);
}

void s72::LED::set_blinkspeed(unsigned long durationOnMs, unsigned long durationOffMs){
  this->durationOnMs = durationOnMs;
  this->durationOffMs = durationOffMs;
}

void s72::LED::set_color(bool const red, bool const green, bool const blue){
  this->red = red;
  this->green = green;
  this->blue = blue;

  // ggf. ist die LED schon an, dann sofort die Farbe wechseln
  switch(this->current_state)
  {
    case s72::LEDState::turn_on:
    case s72::LEDState::on:
    case s72::LEDState::start_blinking_on:
    case s72::LEDState::blinking_on:
    {
      on();
    }
  }
}

void s72::LED::set_color_red() {
  Serial.println(F("LED: red"));
  set_color(true, false, false);
}

void s72::LED::set_color_green() {
  Serial.println(F("LED: green"));
  set_color(false, true, false);
}

void s72::LED::set_color_yellow() {
  Serial.println(F("LED: yellow"));
  set_color(true, true, false);
}

void s72::LED::set_color_blue() {
  Serial.println(F("LED: blue"));
  set_color(false, false, true);
}

void s72::LED::set_color_white() {
  Serial.println(F("LED: white"));
  set_color(true, true, true);
}

void s72::LED::on(){
  set(this->red, this->green, this->blue);
}

void s72::LED::off(){
  set(false, false, false);
}

void s72::LED::set(bool const red, bool const green, bool const blue) {
  digitalWrite(this->pinRed, red ? LOW : HIGH);
  digitalWrite(this->pinGreen, green ? LOW : HIGH);
  digitalWrite(this->pinBlue, blue ? LOW : HIGH);
}

void s72::LED::setup(){
  pinMode(this->pinRed, OUTPUT);
  pinMode(this->pinGreen, OUTPUT);
  pinMode(this->pinBlue, OUTPUT);
  off();
  Serial.println(F("LED setup done (LED is off)"));
  Serial.print(F("  pins: r "));
  Serial.print(this->pinRed);
  Serial.print(F(" g "));
  Serial.print(this->pinGreen);
  Serial.print(F(" b "));
  Serial.println(this->pinBlue);
}

void s72::LED::switchToState(LEDState const new_state) {
  switchToState(new_state, millis());
}

void s72::LED::switchToState(LEDState const new_state, unsigned long const time) {
  this->current_state = new_state;
  this->current_state_start_time = time;

#ifdef DEBUG
  Serial.print(F("LED::switchToState "));
  printStateName(new_state);
  Serial.println();
#endif
}

void s72::LED::update(){

  s72::LEDState next_state = this->current_state;

  switch (this->current_state)
  {
    case s72::LEDState::turn_on:
    {
      on();
      next_state = s72::LEDState::on;
      break;
    }

    case s72::LEDState::on:
    {
      break;
    }

    case s72::LEDState::turn_off:
    {
      off();
      next_state = s72::LEDState::off;
      break;
    }

    case s72::LEDState::off:
    {
      break;
    }

    case s72::LEDState::start_blinking_on:
    {
      on();
      next_state = s72::LEDState::blinking_on;
      break;
    }

    case s72::LEDState::blinking_on:
    {
      if (millis() - this->current_state_start_time > this->durationOnMs) {
        next_state = s72::LEDState::start_blinking_off;
      }
      break;
    }

    case s72::LEDState::start_blinking_off:
    {
      off();
      next_state = s72::LEDState::blinking_off;
      break;
    }

    case s72::LEDState::blinking_off:
    {

      if (millis() - this->current_state_start_time > this->durationOffMs) {
        // ggf. Anzahl der noch zu gebenenden Blinksignale um eins reduzieren
        if (!this->infinite_blink) {
          if (this->repetitions == 0) break;
          this->repetitions--;
        }
        next_state = s72::LEDState::start_blinking_on;
      }
      break;
    }

  }

  if (next_state != this->current_state) {
    switchToState(next_state);
  }

}
