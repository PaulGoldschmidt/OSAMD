#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define lenght 16.0
double percent;

const byte buzzer = 6; //buzzer to arduino pin 5
const byte LED_red = 9;
const byte LED_blue = 10;
const byte LED_green = 11;
const byte MQ135 = A0;
const byte buttonPin = A6;     // the number of the pushbutton pin

void setup() {
  // Serielle Kommunikation zum PC über 9600 baud/sekunde:
  Serial.begin(9600);
  Setup_I2C();
  pinMode(buzzer, OUTPUT); // Die folgenden Pins als Ausgang initalisieren:
  pinMode(LED_red, OUTPUT);
  pinMode(LED_green, OUTPUT);
  pinMode(LED_blue, OUTPUT);
  digitalWrite(LED_red, HIGH); //alle LED-Pins auf "HIGH" setzten, so ist die LED aus.
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, HIGH);
  preheating();
}

void loop() {
  int sensorValue = analogRead(A0);
  Serial.print("Aktuelle Luftqualität: ");
  Serial.println(sensorValue);
  //delay(1000);
  //tone(buzzer, 440); // Send 440 Hz sound signal...
  //delay(1000);        // ...for 1 sec
  //noTone(buzzer);     // Stop sound...
  //delay(1000);        // ...for 1sec
  LCD_Draw();
  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren
  while (buttonvalue() == true) { //Wenn der Button gedrückt ist, wird in diese Funktion gegangen
    Serial.println("Knopf gedrückt!");
    millis100pressed++;
    delay(100);
    if (millis100pressed >= 40) {
      Serial.println("Knopf 4 Sekunden am Stück gedrückt");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ueberspringe");
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
void preheating() { // Die Vorheizschleife
  bool skipPreheating = false;
  LCD_Startup();
  unsigned long startOfPreheating = millis(); //akutelle Laufzeit des Microcontrollers speichern.
  Serial.print("Heize den Sensor vor... Aktuelle Laufzeit: ");
  Serial.print(startOfPreheating);
  Serial.println(" ms.");
  digitalWrite(LED_green, HIGH); // LED wieder aus
  digitalWrite(LED_blue, HIGH);
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
    percent = prozentwert;
    LCD_Draw(); // Die Funktion "LCD_Draw" Aufrufen
    while (buttonvalue() == true) { //Wenn der Button gedrückt ist, wird in diese Funktion gegeangen
      Serial.println("Knopf gedrückt!");
      millis100pressed++;
      delay(100);
      if (millis100pressed >= 40) {
        Serial.println("Knopf 4 Sekunden am Stück gedrückt, überspringe Vorwärmen.");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ueberspringe");
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
