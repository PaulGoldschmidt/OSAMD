#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define lenght 16.0 //LCD Charakterlänge

const byte buzzer = 6; //buzzer to arduino pin 5
const byte LED_red = 9;
const byte LED_green = 10;
const byte LED_blue = 12;

void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Setup_I2C();
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 5 as an output
  pinMode(LED_red, OUTPUT); // Set buzzer - pin 5 as an output
  pinMode(LED_green, OUTPUT); // Set buzzer - pin 5 as an output
  pinMode(LED_blue, OUTPUT); // Set buzzer - pin 5 as an output
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
  digitalWrite(LED_red, HIGH);
  LCD_Draw();
}
