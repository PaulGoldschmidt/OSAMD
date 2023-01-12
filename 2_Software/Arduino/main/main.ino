#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LiquidCrystal I2C - Frank de Brabander - Version 1.1.2
#include <EEPROM.h>
#include <MQUnifiedsensor.h>
#define TEMPINSTALLED // auskommentieren, wenn kein DHT-Temperatursensor verwendet wird.

#ifdef TEMPINSTALLED
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11 //alternativ je nach verwendetem Sensor: DHT22 (auch (AM2302), AM2321) und DHT21
DHT dht(DHTPIN, DHTTYPE);
bool tempinfahrenheit = false; //bei true: Temperatur wird in Fahrenheit ausgegeben!
#endif

//Definitions for MQ135 Sensor
#define placa "Arduino NANO"
#define Voltage_Resolution 5
#define pin A0 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 10 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  

// I2C-Adresse des Display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define lengh 16.0
double percentDirty;

// Einstellungen für den Betrieb der Sensoreinheit
#define maximalwert 768 //Die maximal erlaubte Schlecht-Luftqualität (Standard: 512, Maximal: 1024, Grundwert des Sensors bei sehr guter Luft: circa 250)
//TODO: Aktive Kalibration des Sensors

const byte buzzer = 6;
const byte LED_red = 9;
const byte LED_blue = 10;
const byte LED_green = 11;
const byte buttonPin = A6;

float calcR0 = 0;

bool filpbit = false; // ein Bit was flippt, wenn die rote Stufe erreicht ist. Damit wird die LED zum Blinken gebracht.
byte counter_buzzeralarm = 0; // ein Byte welches hochgezählt wird, damit bei der roten Stufe einmal Pro minute der Buzzer piepst (wenn aktiviert)
bool buzzer_active = true;
bool I2C_backlight = true;
bool calibration_tasked = false;

//Declare Sensor
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

void setup() {
  // Serielle Kommunikation zum PC über 9600 baud/sekunde:
  Serial.begin(9600);
  buzzer_active = EEPROM.read(0); //Lese aus den nichtflüchtigen Speicher die Konfigurationsvariablen
  I2C_backlight = EEPROM.read(1);
  calibration_tasked = EEPROM.read(2);
  EEPROM.get(3, calcR0);
  if (isnan(calcR0)) sensorcalibration();
  Setup_I2C();
  Serial.print(F("Gelesen aus dem EEPROM: Benachrichtigungstöne: "));
  Serial.print(buzzer_active ? "EIN" : "AUS");
  Serial.print(F(" | Hintergrundbeleuchtung des Displays: "));
  Serial.print(I2C_backlight ? "EIN" : "AUS");
  Serial.print(F(" | Kalibration Benötigt: "));
  Serial.print(calibration_tasked ? "JA" : "NEIN");
  Serial.print(F(" | MQ135-Kalibrationswert: "));
  Serial.println(calcR0);
  pinMode(buzzer, OUTPUT); // Die folgenden Pins als Ausgang initalisieren:
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(LED_blue, OUTPUT);
  LED_off();
  #ifdef TEMPINSTALLED
  dht.begin();
  #endif
  //Set math model to calculate the PPM concentration and the value of constants
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(102.2); MQ135.setB(-2.473); // Configure the equation to to calculate NH4 concentration
  MQ135.init(); 
  if (buttonvalue() == false) {
    preheating();
  }
}

void loop() {
  bool displaytemp = false;
  float h;
  float t;
  float cFactor = 0;
  #ifdef TEMPINSTALLED //wenn ein Temperatursensor installiert ist, dann wird hier die aktuelle Temperatur ausgelesen.
  h = dht.readHumidity();
  t = dht.readTemperature(tempinfahrenheit);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Konnte nicht vom Temperatursensor lesen."));
    displaytemp = false;
  }
  else {
    displaytemp = true;
  }
  #endif
  MQ135.update();
  float air_quality = MQ135.readSensor(false, cFactor);
  MQ135.serialDebug();
  const int maxCO2e = 2500; // minimale CO2e-Konzentration in der Luft für 0% Luftqualität
  const int LevelGreenCO2e = 1000; // bis 1000 ppm ist alles im grünen Bereich
  const int LevelYellowCO2e = 1400; // bis 1400 ppm gelber bereich (und alles darüber rot)
  float air_quality_percent = (air_quality / maxCO2e) * 100;
  Serial.print("Aktuelle Luftqualität: ");
  Serial.print(analogRead(pin));
  Serial.print(", das entspricht ");
  Serial.print(air_quality);
  Serial.println(" PPM CO2e. ");
  Serial.print("Ein Temperatursensor wurde installiert: ");
  Serial.println(displaytemp ? "JA" : "NEIN");

  if (displaytemp) { //wenn Temperatur gelesen werden kann, dann dies im seriellen Monitor ausgeben.
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));

    lcd.setCursor(0, 0);
    lcd.print("LQ: ");
    
    if (air_quality <= 1000) {
      lcd.print("");
    }

    else if ((air_quality >= 1000) && (air_quality <= 2000)) {
      lcd.print("");
    }

    else {
        lcd.print("");
    }
    lcd.print(round(air_quality));
    lcd.print(" ppm-e");
    lcd.setCursor(0, 1);
    lcd.print("T: ");
    lcd.print(round(t));
    lcd.print((char)223);
    lcd.print("C | H: "); 
    lcd.print(round(h));
    lcd.print("%"); 
  } 

  else {
    lcd.setCursor(0, 0);
    lcd.print("CO2-e: ");
    lcd.print(air_quality);
    LCD_Draw(air_quality);
  }

  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren / zurücksetzen
  while (buttonvalue() == true) { //Wenn der Button gedrückt ist, wird in diese Funktion gegangen
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
  byte LevelGreenPercent = (LevelGreenCO2e/maxCO2e) * 100;
  byte LevelYellowPercent = (LevelYellowCO2e/maxCO2e) * 100;
  if (percentDirty <= LevelGreenPercent) { // bis 1000 ppm bei 2500 ppm Maximalmaß
    //bis 60% des Maximalwerts ist die Luftqualität gut
    digitalWrite(LED_green, LOW); //Damit ist die LED Grün
    Serial.print("Damit ist die Luftqualität im grünen Bereich, da nicht mehr als ");
    Serial.print(LevelGreenPercent);
    Serial.println("ppm in der Luft sind.");
  }
  else if ((percentDirty >= LevelGreenPercent) && (percentDirty <= LevelYellowPercent)) {
    //jetzt sollte gelüftet werden, damit die LED Gelb
    digitalWrite(LED_green, LOW);
    digitalWrite(LED_red, LOW);
    Serial.print("Damit ist die Luftqualität im gelben Bereich, da nicht mehr als ");
    Serial.print(LevelYellowPercent);
    Serial.println("ppm in der Luft sind.");
  }
  else {
    //die Luftqualität ist sehr schlecht, es sollte dringend gelüftet werden!
    if (filpbit == false) { //mit diesen Bedingungen binkt die LED
      digitalWrite(LED_red, LOW); //LED Rot
      filpbit = true;
    }
    else {
      digitalWrite(LED_red, HIGH);
      filpbit = false;
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
    while (buttonvalue() == true) { //Wenn der Button gedrückt ist, wird in diese Funktion gegeangen
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
        digitalWrite(LED_red, HIGH); //Die auf Grün umschalten
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

bool buttonvalue() { // Diese Schleife wandelt den analogen Wert des Knopfes in einen wahr/unwahr-Wert um.
  if (analogRead(buttonPin) < 512) return true;
  else return false;
}

void LED_off() {
  digitalWrite(LED_red, HIGH); // LED wieder aus
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, HIGH);
}
