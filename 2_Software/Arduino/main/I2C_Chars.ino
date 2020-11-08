//With help from https://www.electronicsblog.net/
//Arduino LCD horizontal progress bar using custom characters
#include <LiquidCrystal.h>

#define lenght 16.0

double percent=100.0;
unsigned char b;
unsigned int peace;

// custom charaters

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
byte p1[8] = {
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10,
  0x10};

byte p2[8] = {
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18,
  0x18};

byte p3[8] = {
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C,
  0x1C};

byte p4[8] = {
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E,
  0x1E};

byte p5[8] = {
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F,
  0x1F};

void Setup_I2C(){
  // initialize the LCD
  lcd.createChar(0, p1);
  lcd.createChar(1, p2);
  lcd.createChar(2, p3);
  lcd.createChar(3, p4);
  lcd.createChar(4, p5);
  lcd.begin();
  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Starting OSAMD...");
  delay(1000);
}

void LCD_Draw() {

  lcd.setCursor(0, 0);

  //ADC conversion

  unsigned int value = analogRead(A0);

  percent = value/1024.0*100.0;

  lcd.print(value);

  lcd.print(" - ");

  lcd.print(percent);
  lcd.print(" %   ");

  lcd.setCursor(0,1);

  double a=lenght/100*percent;

 // drawing black rectangles on LCD

  if (a>=1) {

    for (int i=1;i<a;i++) {

      lcd.write(4);

      b=i;
    }

    a=a-b;

  }

  peace=a*5;

// drawing charater's colums

  switch (peace) {

  case 0:

    break;

  case 1:
    lcd.write(0);

    break;

  case 2:
    lcd.write(1);
    break;

  case 3:
    lcd.write(2);
    break;

  case 4:
    lcd.write(3);
    break;

  }

//clearing line
  for (int i =0;i<(lenght-b);i++) {

    lcd.print(" ");
  }

  ;
}
