#ifndef OSAMD_MQ135_READER_H_
#define OSAMD_MQ135_READER_H_

#include <Arduino.h>
#include "DHTReader.h"

namespace s72 {


class MQ135Event;

class MQ135Listener {
public:
  MQ135Listener() {}
  virtual ~MQ135Listener() {}
  virtual void onEvent(MQ135Event &event) = 0;  // pure virtual, abstrakte Klasse
};


class MQ135Reader : public DHTListener
{
private:
  float prev_rzero = 0.0;
  float prev_correctedRZero = 0.0;
  float prev_resistance = 0.0;
  float prev_ppm = 0.0;
  float prev_correctedPPM = 0.0;
  bool ignore_preheat = false;
  float temperature = 0.0;
  float humidity = -1.0;

  unsigned long startup_time = 0;
  unsigned long lastReadTime = 0;

  MQ135Reader() = default;

public:

  static unsigned long const PREHEAT_DURATION_MS = 15 * 60 * 1000;  // Vorheizzeit des MQ135 sind 15 Minuten
  static unsigned long const READ_DELAY_MS = 1000;  // Zeit zwischen zwischen zwei Lesevorgängen
  static unsigned long const MIN_PPM_DIFF_FOR_EVENT = 2;

  // Parameters for calculating ppm of CO2 from sensor resistance
  static constexpr float PARA = 116.6020682;
  static constexpr float PARB =   2.769034857;
  /// Parameters to model temperature and humidity dependence
  static constexpr float CORA =   0.00035;
  static constexpr float CORB =   0.02718;
  static constexpr float CORC =   1.39538;
  static constexpr float CORD =   0.0018;
  static constexpr float CORE =  -0.003333333;
  static constexpr float CORF =  -0.001923077;
  static constexpr float CORG =   1.130128205;
  // Atmospheric CO2 level for calibration purposes,
  // from "Globally averaged marine surface monthly mean data"
  // available at https://gml.noaa.gov/ccgg/trends/gl_data.html
  static constexpr float ATMOCO2 = 415.58; // Global CO2 Aug 2022

  static constexpr float VOODOO = pow(ATMOCO2 / PARA, (1.0 / PARB));

  static const int PPM_GOOD_UP_TO = 400;
  static const int PPM_OK_UP_TO = 800;

  byte const pin;
  float const rzero; // Calibration resistance at atmospheric CO2 level
  float const rload; // The load resistance on the board in kOhm
  unsigned long const readDelayMs;
  unsigned long const preheatDelayMs;

  MQ135Reader(byte const pin);
  MQ135Reader(byte const pin, float const rzero, float const rload);
  MQ135Reader(byte const pin, unsigned long const readDelayMs, unsigned long const preheatDelayMs);
  MQ135Reader(byte const pin, float const rzero, float const rload, unsigned long const readDelayMs, unsigned long const preheatDelayMs);

  MQ135Reader(const MQ135Reader &) = delete; // no copying
  MQ135Reader &operator=(const MQ135Reader &) = delete;

  bool is_preheating();
  void ignorePreheat(bool const ignore);
  void ignorePreheat();
  void setup();
  void update();
  void calibrate(float const temperature, float const humidity);

  float const getCorrectionFactor(float const t, float const h);
  float const getResistance();
  float const getCorrectedResistance(float const t, float const h);
  float const getPPM();
  float const getCorrectedPPM(float const t, float const h);
  float const getRZero();
  float const getCorrectedRZero(float const t, float const h);

  // DHTListener
  virtual void onEvent(DHTEvent &event);


};


class MQ135Event
{
  friend class MQ135Reader;

private:
  static MQ135Listener *listener;
  static void raise(byte const pin, float const rzero, float const correctedRZero, float const resistance, float const ppm, float const correctedPPM, bool const preheating);

  MQ135Event(byte const pin, float const rzero, float const correctedRZero, float const resistance, float const ppm, float const correctedPPM, bool const preheating) : 
    pin(pin),
    rzero(rzero),
    correctedRZero(correctedRZero),
    resistance(resistance),
    ppm(ppm),
    correctedPPM(correctedPPM),
    preheating(preheating)
    {}
  MQ135Event() = default;

public:
  static void registerListener(MQ135Listener &listener);
  //static void unregisterListener(MQ135Listener *listener);
  
  byte const pin;
  float const rzero = 0.0;
  float const correctedRZero = 0.0;
  float const resistance = 0.0;
  float const ppm = 0.0;
  float const correctedPPM = 0.0;
  bool const preheating = true;

  MQ135Event(const MQ135Event &) = delete; // no copying
  MQ135Event &operator=(const MQ135Event &) = delete;

};

}


#endif