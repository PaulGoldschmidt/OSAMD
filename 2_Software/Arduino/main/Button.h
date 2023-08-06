#ifndef OSAMD_BUTTON_H_
#define OSAMD_BUTTON_H_


#include <Arduino.h>

namespace s72 {

  class ButtonReleaseEvent;

  class ButtonReleaseListener {
  public:
    ButtonReleaseListener() {}
    virtual ~ButtonReleaseListener() {}
    virtual void onEvent(ButtonReleaseEvent &event) = 0;  // method is pure virtual, class is abstract.
  };

  class ButtonReleaseEvent {
    friend class Button;
  private:
    static ButtonReleaseListener *listener;

    ButtonReleaseEvent(byte const pin, unsigned long const pressed_duration) : pin(pin), pressed_duration(pressed_duration) {}
    ButtonReleaseEvent() = default;

    static void raise(byte const pin, unsigned long const pressed_duration);

  public:
    byte const pin;
    unsigned long const pressed_duration;

    ButtonReleaseEvent(const ButtonReleaseEvent &) = delete; // no copying
    ButtonReleaseEvent &operator=(const ButtonReleaseEvent &) = delete;

    static void registerListener(ButtonReleaseListener &listener);
    //static void unregisterListener(ButtonReleaseListener *listener);
  };


  class Button {
  private:

    bool is_pressed = false;
    unsigned long lastStateChangeTime = 0;
    int currentState = LOW;
    int currentStableState = LOW;
    unsigned long currentStateStartTime = 0;

    Button() = default;

    void init();

  public:
    static int const DEBOUNCE_DEFAULT_MS = 50;

    byte const pin;
    bool const is_analog_pin;
    unsigned int const debounceDelay;
    bool const high_means_pressed;


    Button(byte const pin, bool const is_analog_pin);
    Button(byte const pin, bool const is_analog_pin, unsigned int const debounceDelay);
    Button(byte const pin, bool const is_analog_pin, bool const high_means_pressed);
    Button(byte const pin, bool const is_analog_pin, unsigned int const debounceDelay, bool const high_means_pressed);

    Button(const Button &) = delete; // no copying
    Button &operator=(const Button &) = delete;


    byte getPin() const {
      return this->pin;
    }
    bool isAnalogPin() const {
      return this->is_analog_pin;
    }
    unsigned int getDebounceDelay() const {
      return this->debounceDelay;
    }
    bool highMeansPressed() const {
      return this->high_means_pressed;
    }
    bool const pressed() const {
      return this->is_pressed;
    }
    unsigned long const stateDuration() const;
    void setup();
    void update();
  };

}
#endif
