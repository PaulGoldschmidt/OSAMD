#include "Display.h"
#include "Config.h"

unsigned char const GLYPH_FULL = 0;
unsigned char const GLYPH_ONE_FIFTH = 1;
unsigned char const GLYPH_TWO_FIFTH = 2;
unsigned char const GLYPH_THREE_FIFTH = 3;
unsigned char const GLYPH_FOUR_FIFTH = 4;

unsigned char const ONE_FIFTH = 0x10;
unsigned char const TWO_FIFTH = 0x18;
unsigned char const THREE_FIFTH = 0x1C;
unsigned char const FOUR_FIFTH = 0x1E;
unsigned char const FULL = 0x1F;

unsigned char const CHAR_WIDTH_IN_PIXEL = 5;


s72::Display::Display(byte const i2c_address, byte const width, byte const height) : width(width), height(height), lcd(LiquidCrystal_I2C(i2c_address, width, height)) {
}



void create_glyph(LiquidCrystal_I2C &lcd, unsigned char const number, unsigned char const value) {
  byte char_definition[8];
  for (int i = 0; i < 8; i++) {
    char_definition[i] = value;
  }
  lcd.createChar(number, char_definition);
}



void s72::Display::draw_progress_bar(unsigned char percent_value) {
  draw_progress_bar(percent_value, 1);
}



void s72::Display::draw_progress_bar(unsigned char percent_value, unsigned char row) {

  if (percent_value > 100) percent_value = 100;
  lcd.setCursor(0, row);

  // Breite des Fortschrittsbalkens in Pixeln berechnen
  unsigned int fill_width_pixel = (this->width * CHAR_WIDTH_IN_PIXEL * percent_value) / 100;

  // Anzahl der ganz ausgefüllten Zeichen berechnen und darstellen
  byte char_count = fill_width_pixel / CHAR_WIDTH_IN_PIXEL;
  for (int i = 0; i < char_count; i++) {
    lcd.write(GLYPH_FULL);
  }

  // Anzahl der verbleibenden Pixel berechnen und ggf. darstellen
  const byte fill_rest = fill_width_pixel - char_count * CHAR_WIDTH_IN_PIXEL;
  if (fill_rest > 0) {
    lcd.write(fill_rest);
    char_count++;
  }

  // Rest der Zeile mit Leerzeichen füllen
  for (int i = 0; i < (this->width - char_count); i++) {
    lcd.print(F(" "));
  }
}



void s72::Display::setup(s72::Config &config, s72::LED &led, s72::Buzzer &buzzer, s72::MQ135Reader &mq135reader) {

  this->config = &config;
  this->led = &led;
  this->buzzer = &buzzer;
  this->mq135reader = &mq135reader;

  // Hardware initialisieren
  lcd.init();
  if (this->config->is_backlight_enabled()) {
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }

  if (! this->config->is_buzzer_enabled()) {
    this->buzzer->setSilent(true);
  } 
  
  // Spezielle Zeichen für den Fortschrittsbalken definieren, ein Zeichen ist fünf Pixel breit
  create_glyph(lcd, GLYPH_FULL, FULL);
  create_glyph(lcd, GLYPH_ONE_FIFTH, ONE_FIFTH);
  create_glyph(lcd, GLYPH_TWO_FIFTH, TWO_FIFTH);
  create_glyph(lcd, GLYPH_THREE_FIFTH, THREE_FIFTH);
  create_glyph(lcd, GLYPH_FOUR_FIFTH, FOUR_FIFTH);

  // Startnachricht anzeigen
  current_state = s72::DisplayState::ShowStartScreen;
  current_state_start_time = millis();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(F("Starte OSAMD"));
  lcd.setCursor(4, 1);
  lcd.print(OSAMD_VERSION);
  this->led->set_color_blue();
  this->led->set_on();
  
  // Wenn der Taster gedrückt und wieder losgelassen wird, soll onEvent(ButtonReleaseEvent) aufgerufen werden:
  ButtonReleaseEvent::registerListener(*this);
  // Wenn sich MQ135-Werte ändern, soll onEvent(ButtonReleaseEvent) aufgerufen werden:
  MQ135Event::registerListener(*this);
  // Wenn sich DHT-Werte ändern, soll onEvent(ButtonReleaseEvent) aufgerufen werden:
  DHTEvent::registerListener(*this);

  Serial.println(F("Display setup done, ShowStartScreen"));
}



void s72::Display::update() {

  DisplayState next_state = this->current_state;
  bool check_menu_timeout = false;

  switch (this->current_state)
  {
    case s72::DisplayState::ShowStartScreen:
    {
      unsigned long duration = millis() - this->current_state_start_time;
      if (duration > s72::Display::START_SCREEN_DURATION_MS) {
        next_state = s72::DisplayState::ShowPreheat;
      }
      break;
    }
    case s72::DisplayState::ShowPreheat:
    {
      // nach dem Wechsel einmal den Info-Text zum Aufheizen anzeigen
      if (this->prev_state != this->current_state) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Heize Sensor,"));
        lcd.setCursor(0, 1);
        lcd.print(F("Dauer: 15 Min."));
        this->led->set_color_white();
        this->led->set_on();
        break;
      }

      // Vorheizen des MQ135-Sensors schon erledigt?
      unsigned long duration = millis(); // Heizen fängt an, sobald der MQ135 Strom bekommt - also mit dem Start des Mikrokontrollers 
      if (duration > s72::MQ135Reader::PREHEAT_DURATION_MS) {
        Serial.println(F("Display::update ShowPreheat done"));
        next_state = s72::DisplayState::InitShowReadings;
        break;
      }

      // Vorheizzeit noch nicht beendet, wieviele Prozent sind bereits verstrichen?
      unsigned long prozentwert = (duration * 100 / s72::MQ135Reader::PREHEAT_DURATION_MS);
      lcd.setCursor(0, 0); //Prozentwert auf das LCD Plotten
      lcd.print("Vorheizen: ");
      lcd.print(prozentwert);
      lcd.print(" %");
      draw_progress_bar((unsigned char)prozentwert);
      break;
    }
    case s72::DisplayState::InitShowReadings:
    {
      // einmalig zwischengespeicherte Werte darstellen
      Serial.println(F("Display::update InitShowReadings (show cached values)"));
      lcd.clear();
      displayPpm(this->cached_ppm);
      if (this->cached_hum > -1.0) displayHumTemp(this->cached_temp, this->cached_hum);
      next_state = s72::DisplayState::ShowReadings;
      break;
    }
    case s72::DisplayState::ShowReadings:
    {
      // Aktualisierung der Anzeige wird durch Events ausgelöst
      break;
    }
    case s72::DisplayState::ShowConfigBuzzer:
    {
      // nach dem Wechsel einmal den Info-Text zur Buzzer-Konfiguration anzeigen
      bool show_all = (this->prev_state != this->current_state);
      if (show_all) {
        Serial.println(F("Display::update ShowConfigBuzzer: show_all"));
        this->buzzer->keysound(440, 300);
        this->led->set_color_blue();
        this->led->set_on();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Hinweist\357ne:")); //Hinweistöne, mit Umlaut als \357 kodiert
      }
      if (show_all || update_buzzer_option) { // update_buzzer_option wird in onEvent(ButtonReleaseEvent) gesetzt
        if (update_buzzer_option) {
          // Bestätigungston für Änderung
          buzzer->keysound(880, 600);
          Serial.println(F("Display::update ShowConfigBuzzer: update_buzzer_option"));
        }
        lcd.setCursor(0, 1);
        lcd.print(this->config->is_buzzer_enabled() ? F("aktiviert") : F("stumm    ")); // Leerzeichen hinter stumm, um "aktiviert" komplett zu überschreiben
        update_buzzer_option = false;
      }
      check_menu_timeout = true;
      break;
    }
    case s72::DisplayState::ShowConfigBacklight:
    {
      // nach dem Wechsel einmal den Info-Text zur Displaybeleuchtung anzeigen
      bool show_all = (this->prev_state != this->current_state);
      if (show_all) {
        Serial.println(F("Display::update ShowConfigBacklight: show_all"));
        this->buzzer->keysound(440, 300);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("Displaylicht:"));
      }
      if (show_all || update_backlight_option) { // update_backlight_option wird in onEvent(ButtonReleaseEvent) gesetzt
        if (update_backlight_option) {
          // Bestätigungston für Änderung
          this->buzzer->keysound(880, 600);
          Serial.println(F("Display::update ShowConfigBacklight: update_backlight_option"));
        }
        lcd.setCursor(0, 1);
        lcd.print(this->config->is_backlight_enabled() ? F("ein") : F("aus"));
        update_backlight_option = false;
      }
      check_menu_timeout = true;
      break;
    }
  }

  if (check_menu_timeout) {
    unsigned long duration = millis() - this->current_state_start_time;
      if (duration > s72::Display::CONFIG_TIMEOUT_MS) {
        next_state = s72::DisplayState::InitShowReadings;
        Serial.println(F("Display::update timeout in menu"));
      }
  }

  switchToState(next_state);
  this->prev_state = this->current_state;
}



void s72::Display::switchToState(DisplayState new_state) {
  switchToState(new_state, millis());
}



void s72::Display::switchToState(DisplayState new_state, unsigned long time) {
  if (new_state == this->current_state) return;
  this->prev_state = this->current_state;
  this->current_state = new_state;
  this->current_state_start_time = time;

#ifdef DEBUG
  Serial.print(F("Display::switchToState "));
  switch (current_state)
  {
    case s72::DisplayState::ShowStartScreen:
      Serial.println(F("ShowStartScreen"));
      break;
    case s72::DisplayState::ShowPreheat:
      Serial.println(F("ShowPreheat"));
      break;
    case s72::DisplayState::InitShowReadings:
      Serial.println(F("InitShowReadings"));
      break;
    case s72::DisplayState::ShowReadings:
      Serial.println(F("ShowReadings"));
      break;
    case s72::DisplayState::ShowConfigBuzzer:
      Serial.println(F("ShowConfigBuzzer"));
      break;
    case s72::DisplayState::ShowConfigBacklight:
      Serial.println(F("ShowConfigBacklight"));
      break;
  }
#endif

}

void s72::Display::displayHumTemp(float const temp, float const hum) {
  lcd.setCursor(0, 1);
  lcd.print(F("T:"));
  if (isnan(temp)) {
    lcd.print(F("--.-"));
  } else {
    int full_degrees = temp;
    unsigned char centi_degrees = (int)(temp * 10) % 10;
    lcd.print(full_degrees);
    lcd.print(F("."));
    lcd.print(centi_degrees);
  }
  lcd.write(s72::Display::GLYPH_DEGREE);
  lcd.print(F("C, H:"));
  if (isnan(hum)) {
    lcd.print(F("--"));
  } else {
    lcd.print((int)hum);
  }
  lcd.print(F("%"));
}


void s72::Display::displayPpm(float const ppm) {
    byte rest = LCD_WIDTH; // beinhaltet, wie viele Zeichen noch in der Zeile gelöscht werden müssen
    lcd.setCursor(0, 0);

#ifdef DHTTYPE
  lcd.print(F("LQ: "));
#else
  lcd.print(F("Luftqualit\xE1t:"));
  lcd.setCursor(0, 1);
#endif
  
  if (ppm <= s72::MQ135Reader::PPM_GOOD_UP_TO) {
    lcd.print(F("GUT "));
    rest -= 5; // "GUT " + mindestens eine Ziffer
    this->led->set_color_green();
    this->led->set_on();
  } else if (ppm <= s72::MQ135Reader::PPM_OK_UP_TO) {
    lcd.print(F("WARN "));
    rest -= 6;
    this->led->set_color_yellow();
    this->led->set_on();
  } else {
    lcd.print(F("L\365FTEN "));
    rest -= 7;
    this->led->set_color_red();
    this->led->set_blinking();
    this->buzzer->warn(440, 1000, 59000, 100000); // jede Minute eine Sekunde hupen
  }
  int ppm_out = (int)ppm;
  if (ppm_out < 0) ppm_out = 0; // Wokwi-Artefakt
  // Anzahl Ziffern feststellen
  int ppm_lenght_test = ppm_out;
  while ( ppm_lenght_test /= 10 ) {
    rest--;
  }
  lcd.print(ppm_out);
  lcd.print(F("ppm"));
  // restliche Zeichen der Zeile überschreiben
  while (rest--) {
    lcd.print(F(" "));
  }
}


// MQ135Listener
void s72::Display::onEvent(MQ135Event &event) {
  this->cached_ppm = event.ppm;
  displayPpm(this->cached_ppm);
#ifdef DEBUG
  Serial.print(F("Display: MQ135Event received, ppm: "));
  Serial.println(this->cached_ppm);
  Serial.println(F("  value sent to lcd"));
  Serial.println(F("Display: MQ135Event done"));
#endif
}

// DHTListener

void s72::Display::onEvent(s72::DHTEvent &event) {
#ifdef DEBUG
  Serial.print(F("Display: DHTEvent received, T: "));
  Serial.print(event.temperature);
  Serial.print(" H: ");
  Serial.println(event.temperature);
#endif

  if (!isnan(event.temperature)) this->cached_temp = event.temperature;
  if (!isnan(event.humidity)) this->cached_hum = round(event.humidity);
  if (this->current_state != s72::DisplayState::ShowReadings) {
#ifdef DEBUG
    Serial.println(F("  values cached"));
#endif
    return;
  }
  displayHumTemp(event.temperature, event.humidity);

#ifdef DEBUG
  Serial.println(F("  values sent to lcd"));
  Serial.println(F("Display: DHTEvent done"));
#endif
}


// ButtonReleaseListener

void s72::Display::onEvent(ButtonReleaseEvent &event) {
  bool is_long_press = event.pressed_duration >= LONG_KEYPRESS_MS;

#ifdef DEBUG
  Serial.print(F("Display: ButtonReleaseEvent"));
  Serial.print("  dur: ");
  Serial.print(event.pressed_duration);
  Serial.print("  is_long: ");
  Serial.println(is_long_press);

  Serial.print(F("  in Status "));
  switch (this->current_state)
  {
    case s72::DisplayState::ShowStartScreen:
      Serial.println(F("ShowStartScreen"));
      break;
    case s72::DisplayState::ShowPreheat:
      Serial.println(F("ShowPreheat"));
      break;
    case s72::DisplayState::ShowReadings:
      Serial.println(F("ShowReadings"));
      break;
    case s72::DisplayState::ShowConfigBuzzer:
      Serial.println(F("ShowConfigBuzzer"));
      break;
    case s72::DisplayState::ShowConfigBacklight:
      Serial.println(F("ShowConfigBacklight"));
      break;
  }

#endif

  switch (this->current_state)
  {
    case s72::DisplayState::ShowStartScreen:
    {
      Serial.println(F("  skip start screen"));
      switchToState(s72::DisplayState::ShowPreheat);
      break;
    }


    case s72::DisplayState::ShowPreheat:
    {
      if (is_long_press) {
        Serial.println(F("  skip preheating"));
        this->mq135reader->ignorePreheat(); // damit der mq135reader Events auslöst, auch wenn die Vorheizzeit noch nicht herum ist
        switchToState(s72::DisplayState::InitShowReadings);
      } else {
        Serial.println(F("  ignored"));
      }
      break;
    }


    case s72::DisplayState::ShowReadings:
    {
      Serial.println(F("  start config menu"));
      switchToState(s72::DisplayState::ShowConfigBuzzer);
      break;
    }


    case s72::DisplayState::ShowConfigBuzzer:
    {
      if (is_long_press) {
        Serial.println(F("  toggle is_buzzer_enabled"));
        this->config->toggle_buzzer_enabled();
        this->buzzer->setSilent(!this->config->is_buzzer_enabled());
        this->update_buzzer_option = true;
      } else {
        Serial.println(F("  skip to ShowConfigBacklight"));
        switchToState(s72::DisplayState::ShowConfigBacklight);
      }
      break;
    }


    case s72::DisplayState::ShowConfigBacklight:
    {
      if (is_long_press) {
        Serial.println(F("  toggle is_backlight_enabled"));
        this->config->toggle_backlight_enabled();
        if (this->config->is_backlight_enabled()) {
          lcd.backlight();
        } else {
          lcd.noBacklight();
        }
        this->update_backlight_option = true;
      } else {
        Serial.println(F("  save config to EEPROM"));
        this->config->save();
        Serial.println(F("  skip to InitShowReadings"));
        switchToState(s72::DisplayState::InitShowReadings);
      }
      break;
    }


    default:
    {
      Serial.println(F("  ignored"));
      break;
    }
  }

#ifdef DEBUG
  Serial.println(F("Display: ButtonReleaseEvent done"));
#endif


}
