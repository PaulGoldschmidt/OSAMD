#ifndef OSAMD_BUZZER_H_
#define OSAMD_BUZZER_H_

#include <Arduino.h>
#include "Config.h"

namespace s72 {

// mögliche Zustände des Buzzers
enum class BuzzerState
{
  idle,
  start_sound,
  sound_is_on,
  end_sound,
  sound_is_off,
  sound_done
};

class Buzzer
{

private:
  BuzzerState current_state = BuzzerState::idle;
  unsigned long current_state_start_time = 0;
  unsigned int frequency = 0;
  unsigned long durationOn = 0;
  unsigned long durationOff = 0;
  unsigned int repetitions = 0;

  int const pwm_channel = 0;

  bool silent = false;

  Buzzer() = default;

  void switchToState(BuzzerState const new_state);
  void switchToState(BuzzerState const new_state, unsigned long const time);

  void hw_sound(unsigned int const freq) const;
  void hw_noSound() const;


public:
  byte const pin;
  
  Buzzer(byte const pin);
  Buzzer(const Buzzer &) = delete; // no copying
  Buzzer &operator=(const Buzzer &) = delete;

  void warn(unsigned int const frequency, unsigned long const durationOn, unsigned long const durationOff, unsigned int const repetitions);
  void keysound(unsigned int const frequency, unsigned long const duration);
  void setSilent(bool const state);
  void setup();
  void update();
};

}
#endif