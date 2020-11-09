#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

const int buzzer = 5; //buzzer to arduino pin 5

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Setup_I2C();
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 5 as an output
}

void loop() {
  int sensorValue = analogRead(A0);
  // print out the value you read:
  Serial.println(sensorValue);
//  delay(1000);
//  tone(buzzer, 440); // Send 440 Hz sound signal...
//  delay(1000);        // ...for 1 sec
//  noTone(buzzer);     // Stop sound...
//  delay(1000);        // ...for 1sec
LCD_Draw();
}
