#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LiquidCrystal I2C - Frank de Brabander - Version 1.1.2
#include <EEPROM.h>

/*
 * ----------------------
 * Start Codekonfiguration
 * ======================
 */

// Für welche Platine soll der Code kompiliert werden (Arduino Nano, WEMOS D1 Mini light oder (ESP32)?
//#define ARDUINONANO
#define ESP8266
//#define ESP32

// Ist auf der Platine ein Temperatur- und Luftfeuchtesensor verbaut?
#define TEMPINSTALLED // auskommentieren, wenn kein DHT-Temperatursensor verwendet wird.
#ifdef TEMPINSTALLED
  // Welcher Sensor ist verwendetet: DHT11, DHT22 (auch (AM2302), AM2321) und DHT21?
  #define DHTTYPE DHT22
#endif

// Einstellungen für den Betrieb der MQ 135-Sensoreinheit
#define BAD_AIR_VALUE 768 //Die maximal erlaubte Schlecht-Luftqualität (Standard: 512, Maximal: 1024, Grundwert des Sensors bei sehr guter Luft: circa 250)
#define PERCENT_GOOD 65
#define PERCENT_OK 75

/* 
 * ======================
 * Ende Codekonfiguration
 * ----------------------
*/

#ifdef TEMPINSTALLED
  #include <DHT.h>
  #ifdef ARDUINONANO
    #define DHTPIN 2
  #endif
  #ifdef ESP8266
    #define DHTPIN D7
  #endif
  DHT dht(DHTPIN, DHTTYPE);
  bool tempinfahrenheit = false; //bei true: Temperatur wird in Fahrenheit ausgegeben!
#endif

// I2C-Adresse des Display
#define LCD_I2C_ADDRESS 0x27
#define LCD_WIDTH 16
#define LCD_HEIGHT 2
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_WIDTH, LCD_HEIGHT);

#define lengh 16.0
double percentDirty = 0;

//TODO: Aktive Kalibrierung des Sensors


#ifdef ESP8266
  const byte buzzer = D4;
  const byte LED_red = D5;
  const byte LED_blue = D6;
  const byte LED_green = D3;
  const byte MQ135 = A0;
  const byte buttonPin = D8; // the number of the pushbutton pin
#endif
#ifdef ARDUINONANO
  const byte buzzer = 6; // buzzer to arduino pin 6
  const byte LED_red = 9;
  const byte LED_blue = 10;
  const byte LED_green = 11;
  const byte MQ135 = A0;
  const byte buttonPin = A6; // the number of the pushbutton pin
#endif

bool flipBit = false; // ein Bit was flippt, wenn die rote Stufe erreicht ist. Damit wird die LED zum Blinken gebracht.
byte counter_buzzeralarm = 0; // ein Byte welches hochgezählt wird, damit bei der roten Stufe einmal pro Minute der Buzzer piepst (wenn aktiviert)
bool buzzer_active = true;
bool I2C_backlight = true;
String const OSAMD_VERSION = "V. 1.2A";

void setup() {
  // Serielle Kommunikation zum PC über 9600 baud/sekunde:
  Serial.begin(9600);
  
  // FIXME: Beim ersten Start kann im EEPROM noch kein sinnvoller Wert stehen
  buzzer_active = EEPROM.read(0); //Lese aus den nichtflüchtigen Speicher die Konfigurationsvariablen
  I2C_backlight = EEPROM.read(1);
  
  Setup_I2C();
  Serial.print(F("Gelesen aus dem EEPROM: Benachrichtigungstöne: "));
  Serial.print(buzzer_active ? "EIN" : "AUS");
  Serial.print(F(" | Hintergrundbeleuchtung des Displays: "));
  Serial.println(I2C_backlight ? "EIN" : "AUS");

  // Pins für LED und Buzzer als Ausgang initalisieren
  pinMode(buzzer, OUTPUT);
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(LED_blue, OUTPUT);
  
  LED_off();
  
  #ifdef TEMPINSTALLED
    dht.begin();
  #endif
  
  if (is_button_pressed() == false) {
    preheating();
  }

  #ifdef ESP8266
    //WLAN initialisieren
  #endif

}

void loop() {
  #ifdef TEMPINSTALLED //wenn ein Temperatursensor installiert ist, dann wird hier die aktuelle Temperatur ausgelesen.
  float h = dht.readHumidity();
  float t = dht.readTemperature(tempinfahrenheit);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Konnte nicht vom Temperatursensor lesen."));
    return;
  //} else {
    //displaytemp = true;
  }
  #endif
  
  int mq_val = analogRead(A0);
  percentDirty = ((float)mq_val / (float)BAD_AIR_VALUE) * 100; // Den Prozentwert des Maximalwertes
  float percentClean = 100 - percentDirty; // hier so, dass niedrigere Werte besser sind
  Serial.print(F("Aktuelle Luftqualität: "));
  Serial.print(mq_val);
  Serial.print(F(", das entspricht "));
  Serial.print(percentClean);
  Serial.println(F(" %. "));

  #ifdef TEMPINSTALLED

    //wenn Temperatur gelesen werden kann, dann dies im seriellen Monitor ausgeben.
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));

    lcd.setCursor(0, 0);
    lcd.print("LQ: ");
    if (percentDirty <= 65) {
      lcd.print("GUT (");
    } else if (percentDirty <= 75) {
      lcd.print("WARN (");
    } else {
      lcd.print("L\365FTEN (");
    }
    lcd.print(round(percentClean));
    lcd.print(")");
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(round(t));
    lcd.print((char)223);
    lcd.print("C | H: ");
    lcd.print(round(h));
    lcd.print("%");
  #else
    lcd.setCursor(0, 0);
    lcd.print("Luftqual.: ");
    lcd.print(percentClean);
    LCD_Draw(percentClean);
  #endif

  //Taster entprellt abfragen
  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren / zurücksetzen
  while (is_button_pressed()) {
    Serial.println("Knopf gedrückt!");
    millis100pressed++;
    delay(100);
    if (millis100pressed >= 30) {
      LED_off();
      LCD_Config();
      break;
    }
  }
  
  LED_off();
  
  if (percentDirty <= PERCENT_GOOD) {
    //bis 65% des Maximalwerts ist die Luftqualität gut
    set_led_green();
    digitalWrite(LED_green, LOW); //Damit ist die LED Grün
    Serial.println("Damit ist die Luftqualität im grünen Bereich.");
  } else if (percentDirty <= PERCENT_OK) {
    //jetzt sollte gelüftet werden, damit die LED Gelb
    digitalWrite(LED_green, LOW);
    digitalWrite(LED_red, LOW);
    Serial.println("Damit ist die Luftqualität im gelben Bereich.");
  } else {
    //die Luftqualität ist sehr schlecht, es sollte dringend gelüftet werden!
    if (flipBit == false) { //mit diesen Bedingungen binkt die LED
      digitalWrite(LED_red, LOW); //LED Rot
      flipBit = true;
    }
    else {
      digitalWrite(LED_red, HIGH);
      flipBit = false;
    }

    if ((counter_buzzeralarm >= 120) && (buzzer_active == true)) { //und hier wird ein mal pro minute der Buzzer aktiv, wenn eingeschaltet
      tone(buzzer, 440);
      delay(300);
      noTone(buzzer);
      delay(300);
      tone(buzzer, 440);
      delay(300);
      noTone(buzzer);
      counter_buzzeralarm = 0;
    }
    else if (buzzer_active == false) {
      counter_buzzeralarm = 0;
    }
    else counter_buzzeralarm++; //sonst hochzählen
    Serial.println("Damit ist die Luftqualität im roten Bereich.");
  }
  delay(500);
}

void preheating() { // Die Vorheizschleife
  bool skipPreheating = false;
  LCD_Startup();
  unsigned long startOfPreheating = millis(); //akutelle Laufzeit des Microcontrollers speichern.
  Serial.print("Heize den Sensor vor... Aktuelle Laufzeit: ");
  Serial.print(startOfPreheating);
  Serial.println(" ms.");
  LED_off(); // LED wieder aus
  digitalWrite(LED_red, LOW); //Nur Rot bleibt an
  lcd.clear();
  while ((millis() < 900000) && (skipPreheating == false)) {
    int millis100pressed = 0; // Für das Überspringen der Vorheizphase Zählervariable initalisieren
    int prozentwert = (millis() / 9000); //Prozentwert der Vorheizzeit berechnen.
    Serial.print("Der Sensor heizt seit ");
    Serial.print(millis());
    Serial.print(" ms vor, das sind ");
    Serial.print(prozentwert);
    Serial.println(" % der Vorheizphase");
    delay(1000);
    lcd.setCursor(0, 0); //Prozentwert auf das LCD Plotten
    lcd.print("Vorheizen: ");
    lcd.print(prozentwert);
    lcd.print(" %");
    LCD_Draw(prozentwert); // Die Funktion "LCD_Draw" Aufrufen
    while (is_button_pressed()) {
      Serial.println("Knopf gedrückt!");
      millis100pressed++;
      delay(100);
      if (millis100pressed >= 30) {
        Serial.println("Knopf 4 Sekunden am Stück gedrückt, überspringe Vorwärmen.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("\365berspringe"); //Überspringe mit \365 als Ü kodiertem Charakter
        lcd.setCursor(0, 1);
        lcd.print("Vorheizen!");
        digitalWrite(LED_red, HIGH); //Die LED auf Grün umschalten
        digitalWrite(LED_green, LOW);
        delay(1500);
        skipPreheating = true;
        break;
      }
    }
  }
  lcd.clear();
  digitalWrite(LED_red, HIGH); //alle LED-Pins auf "HIGH" setzten, so ist die LED aus.
}

bool is_button_pressed() {
  #ifdef ESP8266
    return (digitalRead(buttonPin) == LOW);
  #endif
  #ifdef ARDUINONANO 
    return (analogRead(buttonPin) < 512);
  #endif
}

void LED_off() {
  digitalWrite(LED_red, HIGH);
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, HIGH);
}

void set_led_red() {
  digitalWrite(LED_red, LOW);
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, HIGH);
}

void set_led_yellow() {
  digitalWrite(LED_red, LOW);
  digitalWrite(LED_green, LOW);
  digitalWrite(LED_blue, HIGH);
}

void set_led_green() {
  digitalWrite(LED_red, HIGH);
  digitalWrite(LED_green, LOW);
  digitalWrite(LED_blue, HIGH);
}
