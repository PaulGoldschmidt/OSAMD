#include <Wire.h>
#include <LiquidCrystal_I2C.h> // LiquidCrystal I2C - Frank de Brabander - Version 1.1.2
#include <EEPROM.h>
#define TEMPINSTALLED // auskommentieren, wenn kein DHT-Temperatursensor verwendet wird.

#ifdef TEMPINSTALLED
#include <DHT.h>
#define DHTPIN 2
#define DHTTYPE DHT11 //alternativ je nach verwendetem Sensor: DHT22 (auch (AM2302), AM2321) und DHT21
DHT dht(DHTPIN, DHTTYPE);
bool tempinfahrenheit = false; //bei true: Temperatur wird in Fahrenheit ausgegeben!
#endif

// I2C-Adresse des Display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define lengh 16.0
double percentDirty;

// Einstellungen für den Betrieb der Sensoreinheit
#define maximalwert 768 //Die maximal erlaubte Schlecht-Luftqualität (Standard: 512, Maximal: 1024, Grundwert des Sensors bei sehr guter Luft: circa 250)
//TODO: Aktive Kalibration des Sensors

const byte buzzer = 6; //buzzer to arduino pin 6
const byte LED_red = 9;
const byte LED_blue = 10;
const byte LED_green = 11;
const byte MQ135 = A0;
const byte buttonPin = A6; // the number of the pushbutton pin

bool filpbit = false; // ein Bit was flippt, wenn die rote Stufe erreicht ist. Damit wird die LED zum Blinken gebracht.
byte counter_buzzeralarm = 0; // ein Byte welches hochgezählt wird, damit bei der roten Stufe einmal Pro minute der Buzzer piepst (wenn aktiviert)
bool buzzer_active = true;
bool I2C_backlight = true;

void setup() {
  // Serielle Kommunikation zum PC über 9600 baud/sekunde:
  Serial.begin(9600);
  buzzer_active = EEPROM.read(0); //Lese aus den nichtflüchtigen Speicher die Konfigurationsvariablen
  I2C_backlight = EEPROM.read(1);
  Setup_I2C();
  Serial.print("Gelesen aus dem EEPROM: Benachrichtigungstöne: ");
  Serial.print(buzzer_active ? "EIN" : "AUS");
  Serial.print(" | Hintergrundbeleuchtung des Displays: ");
  Serial.println(I2C_backlight ? "EIN" : "AUS");
  pinMode(buzzer, OUTPUT); // Die folgenden Pins als Ausgang initalisieren:
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(LED_blue, OUTPUT);
  LED_off();
  #ifdef TEMPINSTALLED
  dht.begin();
  #endif
  if (buttonvalue() == false) {
    preheating();
  }
}

void loop() {
  bool displaytemp = false;
  #ifdef TEMPINSTALLED //wenn ein Temperatursensor installiert ist, dann wird hier die aktuelle Temperatur ausgelesen.
  float h = dht.readHumidity();
  float t = dht.readTemperature(tempinfahrenheit);
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Konnte nicht vom Temperatursensor lesen."));
    return;
    displaytemp = false;
  }
  else {
    displaytemp = true;
  }
  #endif
  percentDirty = ((float)analogRead(A0) / (float)maximalwert) * 100; // Den Prozentwert des Maximalwertes
  float percentClean = 100 - percentDirty; //hier so, dass niedrigere Werte besser sind
  Serial.print("Aktuelle Luftqualität: ");
  Serial.print(analogRead(A0));
  Serial.print(", das entspricht ");
  Serial.print(percentClean);
  Serial.println(" %. ");
  Serial.print("Ein Temperatursensor wurde installiert: ");
  Serial.println(displaytemp ? "JA" : "NEIN");

  if (displaytemp) { //wenn Temperatur gelesen werden kann, dann dies im seriellen Monitor ausgeben.
    Serial.print(F("Humidity: "));
    Serial.print(h);
    Serial.print(F("%  Temperature: "));
    Serial.print(t);
    Serial.print(F("°C "));

    lcd.setCursor(0, 0);
    lcd.print("Luftqual.: ");
    lcd.print(percentClean);
    lcd.setCursor(0, 1);
    lcd.print("");
    lcd.print(round(t));
    lcd.print((char)223);
    lcd.print("C | "); 
    lcd.print(round(h));
    lcd.print("%"); 
  } 
  else {
    lcd.setCursor(0, 0);
    lcd.print("Luftqual.: ");
    lcd.print(percentClean);
    LCD_Draw(percentClean);
  }

  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren
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
  if (percentDirty <= 65) {
    Serial.println("Damit ist die Luftqualität im grünen Bereich.");
    //bei 70% des Maximalwerts ist die Luftqualität gut
    digitalWrite(LED_green, LOW); //Damit ist die LED Grün
  }
  else if ((percentDirty >= 65) && (percentDirty <= 75)) {
    Serial.println("Damit ist die Luftqualität im gelben Bereich.");
    //jetzt sollte gelüftet werden, damit die LED Gelb
    digitalWrite(LED_green, LOW);
    digitalWrite(LED_red, LOW);
  }
  else {
    Serial.println("Damit ist die Luftqualität im roten Bereich.");
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
      if (millis100pressed >= 40) {
        Serial.println("Knopf 4 Sekunden am Stück gedrückt, überspringe Vorwärmen.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("\365berspringe"); //Überspringe mit \365 als Ü kodiertem Charakter
        lcd.setCursor(0, 1);
        lcd.print("Vorheizen!");
        digitalWrite(LED_red, HIGH); //Die auf Grün umschalten
        digitalWrite(LED_green, LOW);
        delay(3000);
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
