#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define lenght 16.0 //LCD Charakterlänge

const byte buzzer = 6; //buzzer to arduino pin 5
const byte LED_red = 9;
const byte LED_blue = 10;
const byte LED_green = 11;
const byte MQ135 = A0;

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
  // print out the value you read:
  Serial.println(sensorValue);
  //delay(1000);
  //tone(buzzer, 440); // Send 440 Hz sound signal...
  //delay(1000);        // ...for 1 sec
  //noTone(buzzer);     // Stop sound...
  //delay(1000);        // ...for 1sec
  LCD_Draw();
}
void preheating() {
  lcd.setCursor(0,0);
  lcd.print("Heize Sensor...");
  unsigned long startOfPreheating = millis(); //akutelle Laufzeit des Microcontrollers speichern.
  Serial.print("Heize den Sensor vor... Aktuelle Laufzeit: ");
  Serial.print(startOfPreheating);
  Serial.println(" ms.");
  while ((millis() - startOfPreheating) < 900000) {
    Serial.println(millis() / 1000);
  }
}
