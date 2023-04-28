#include <Arduino.h>
#include "Buzzer.h"
#include "Config.h"

s72::Buzzer::Buzzer(byte const pin) : pin(pin) {}

void s72::Buzzer::sound(unsigned int const freq) {
  if (this->silent) return;
#ifdef ESP32DEVKIT
  ledcAttachPin(this->pin, pwm_channel);
  ledcWriteTone(pwm_channel, freq);
#else
  tone(this->pin, freq);
#endif
}

void s72::Buzzer::noSound() {
#ifdef ESP32DEVKIT
  ledcDetachPin(this->pin);
#else
  noTone(this->pin);
#endif
}


void s72::Buzzer::switchToState(BuzzerState const new_state) {
  switchToState(new_state, millis());
}

void s72::Buzzer::switchToState(BuzzerState const new_state, unsigned long const time) {
  this->current_state = new_state;
  this->current_state_start_time = time;

  #ifdef DEBUG
    Serial.print(F("Buzzer::switchToState "));
  
  switch (current_state)
  {
    case s72::BuzzerState::idle:
      Serial.println(F("idle"));
      break;
    case s72::BuzzerState::start_sound:
      Serial.println(F("start_sound"));
      break;
    case s72::BuzzerState::sound_is_on:
      Serial.println(F("sound_is_on"));
      break;
    case s72::BuzzerState::end_sound:
      Serial.println(F("end_sound"));
      break;
    case s72::BuzzerState::sound_is_off:
      Serial.println(F("sound_is_off"));
      break;
    case s72::BuzzerState::sound_done:
      Serial.println(F("sound_done"));
      break;
  }
#endif

}

void s72::Buzzer::warn(unsigned int const frequency, unsigned long const durationOn, unsigned long const durationOff, unsigned int const repetitions) {
  this->frequency = frequency;
  this->durationOn = durationOn;
  this->durationOff = durationOff;
  this->repetitions = repetitions;
  // Warnton nur neu starten, wenn der Buzzer nicht schon im Warnmodus ist
  if (this->current_state == s72::BuzzerState::idle) {
    switchToState(s72::BuzzerState::start_sound);
  }
}
  
  
void s72::Buzzer::keysound(unsigned int const frequency, unsigned long const duration) {
  this->frequency = frequency;
  this->durationOn = durationOn;
  this->durationOff = 0;
  this->repetitions = 1;
  switchToState(s72::BuzzerState::start_sound);
}


void s72::Buzzer::setSilent(bool const silent) {
  this->silent = silent;
  if (silent) {
    noSound();
  }
}

void s72::Buzzer::setup() {
  pinMode(this->pin, OUTPUT);
#ifdef ESP32DEVKIT
  int const pwm_channel = 0;
  int const pwm_frequency = 2000;
  int const resolution_bits = 12;
  ledcSetup(pwm_channel, pwm_frequency, resolution_bits);
#endif
  noSound();
  this->current_state = s72::BuzzerState::idle;
  Serial.print(F("Buzzer setup done (pin "));
  Serial.print(this->pin);
  Serial.println(F(")"));
}


void s72::Buzzer::update() {
  switch (this->current_state) {

    case s72::BuzzerState::idle:
    {
      // warten auf den nächsten Auftrag ...
      break;
    }
    
    case s72::BuzzerState::start_sound:
    {
      s72::Buzzer::sound(this->frequency);
      switchToState(s72::BuzzerState::sound_is_on);
      break;
    }

    case s72::BuzzerState::sound_is_on:
    {
      if (millis() - this->current_state_start_time < this->durationOn) return;
      switchToState(s72::BuzzerState::end_sound);
      break;
    }

    case s72::BuzzerState::end_sound:
    {
      s72::Buzzer::noSound();
      switchToState(s72::BuzzerState::sound_is_off);
      break;
    }
    case s72::BuzzerState::sound_is_off:
    {
      // nur "warten", wenn eine weitere Ton-Phase folgt
      if (this->repetitions < 2) {
        switchToState(s72::BuzzerState::sound_done);
        break;
      }
      if (millis() - this->current_state_start_time < this->durationOff) return;
      this->repetitions--;
      switchToState(s72::BuzzerState::start_sound);
      break;
    }
    case s72::BuzzerState::sound_done:
    {
      this->frequency = 0;
      this->durationOn = 0;
      this->durationOff = 0;
      this->repetitions = 0;
      switchToState(s72::BuzzerState::idle);
    }
  }



}