#ifndef OSAMD_LED_H_
#define OSAMD_LED_H_

#include <Arduino.h>

namespace s72 {

enum class LEDState
{
  turn_on,
  on,
  turn_off,
  off,
  start_blinking_on,
  blinking_on,
  start_blinking_off,
  blinking_off
};

class LED {
private:
  LEDState current_state = LEDState::turn_off;
  unsigned long current_state_start_time = 0;
  unsigned long durationOnMs = 500;
  unsigned long durationOffMs = 500;
  unsigned int repetitions;
  bool infinite_blink = true;
  bool enabled = true; // nur wenn true wird die LED tatsächlich eingeschaltet

  bool red = false;
  bool green = false;
  bool blue = false;

  void on();
  void off();
  void set(bool const red, bool const green, bool const blue);
  void switchToState(LEDState const new_state);
  void switchToState(LEDState const new_state, unsigned long const time);


  LED() = default;

public:
  byte const pinRed;
  byte const pinGreen;
  byte const pinBlue;
  
  LED(byte const pinRed, byte const pinGreen, byte const pinBlue);

  LED(const LED &) = delete; // no copying
  LED &operator=(const LED &) = delete;

  void set_on();
  void set_off();
  void set_blinking();
  void set_blinking(unsigned int repetitions);
  void set_blinkspeed(unsigned long durationOnMs, unsigned long durationOffMs);
  void set_blink(unsigned int repetitions);
  void set_color(bool const red, bool const green, bool const blue);
  void set_color_red();
  void set_color_green();
  void set_color_yellow();
  void set_color_blue();
  void set_color_white();
  void set_enabled(bool const state);

  void setup();
  void update();

};

}

#endif
