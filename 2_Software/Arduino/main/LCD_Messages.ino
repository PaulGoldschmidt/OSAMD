void LCD_Startup() { //Die LED-Startup-Anzeige
  Serial.println("Starte das OSAMD-Projekt!");
  digitalWrite(LED_green, LOW);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Starte OSAMD");
  lcd.setCursor(4, 1);
  lcd.print(OSAMD_VERSION);
  delay(2500);
  digitalWrite(LED_green, HIGH);
  digitalWrite(LED_blue, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Heize Sensor,");
  lcd.setCursor(0, 1);
  lcd.print("Dauer: 15 Min.");
  delay(2500);
  digitalWrite(LED_green, LOW);
  digitalWrite(LED_blue, LOW);
  digitalWrite(LED_red, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("\365berspringen:"); //Überspringen mit \365 als Ü kodiertem Charakter
  lcd.setCursor(0, 1);
  lcd.print("3 Sek. Dr\365cken"); // Drücken
  delay(3500);
}

void LCD_Config() {
  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren
  unsigned int countertrybutton = 0; //Diese Variable spreichert, wie häufig bereits geprüft wurde, ob der Knopf schon gedrückt wurde.
  bool buttonpressed = false;
  bool buttonpressedlong = false;
  bool changevalue = false;
  Serial.println("Knopf 4 Sekunden am Stück gedrückt");
  lcd.clear();
  if (buzzer_active == true) {
    tone(buzzer, 440);
    delay(300);
    noTone(buzzer);
  }
  lcd.setCursor(0, 0);
  lcd.print("Konfigurations-");
  lcd.setCursor(0, 1);
  lcd.print("Assistent");
  digitalWrite(LED_blue, LOW); //Die auf Blau umschalten
  delay(2700);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hinweist\357ne:"); //Hinweistöne, mit Umlaut als \357 kodiert
  lcd.setCursor(0, 1);
  if (buzzer_active == true) lcd.print("Aktiviert");
  else lcd.print("Stumm");
  while ((countertrybutton <= 200) && (buttonpressedlong == false)) {
    delay(100);
    countertrybutton++;
    Serial.println(countertrybutton);
    while (is_button_pressed() && (buttonpressedlong == false)) { //Wenn der Button gedrückt ist, wird in diese Funktion gegangen
      countertrybutton = 0; //Die Zählervariable wieder auf 0
      Serial.println("Knopf gedrückt!");
      millis100pressed++;
      delay(100);
      if (millis100pressed >= 25) {
        buttonpressedlong = true;
      }
      else {
        changevalue = true;
      }
    }
    if ((buttonpressedlong == false) && (changevalue == true)) {
      changevalue = false;
      millis100pressed = 0;
      buzzer_active = !buzzer_active;
      Serial.println("Invertiere button_active.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Hinweist\357ne:"); //Hinweistöne, mit Umlaut als \357 kodiert
      lcd.setCursor(0, 1);
      if (buzzer_active == true) lcd.print("Aktiviert");
      else lcd.print("Stumm");
    }
  }
  if (buttonpressedlong == false) {
    return;
  }
  buttonpressedlong = false; // Die Variablen wieder zurücksetzen.
  countertrybutton = 0;
  millis100pressed = 0;
  //Menü 2: Hintergrundbeleuchtung
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Displaylicht:"); //Hinweistöne, mit Umlaut als \357 kodiert
  lcd.setCursor(0, 1);
  if (I2C_backlight == true) lcd.print("Eingeschaltet");
  else lcd.print("Ausgeschaltet");
  Serial.println("Menü: Hintergrundbeleuchtung");
  while ((countertrybutton <= 200) && (buttonpressedlong == false)) {
    Serial.println("loop");
    delay(100);
    countertrybutton++;
    Serial.println(countertrybutton);
    while (is_button_pressed() && (buttonpressedlong == false)) { //Wenn der Button gedrückt ist, wird in diese Funktion gegangen
      countertrybutton = 0; //Die Zählervariable wieder auf 0
      Serial.println("Knopf gedrückt!");
      millis100pressed++;
      delay(100);
      if (millis100pressed >= 25) {
        buttonpressedlong = true;
      }
      else {
        changevalue = true;
      }
    }
    if ((buttonpressedlong == false) && (changevalue == true)) {
      changevalue = false;
      millis100pressed = 0;
      I2C_backlight = !I2C_backlight;
      Serial.println("Invertiere I2C_backlight.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Displaylicht:"); //Hinweistöne, mit Umlaut als \357 kodiert
      lcd.setCursor(0, 1);
      if (I2C_backlight == true) {
        lcd.backlight();
        lcd.print("Eingeschaltet");
      }
      else {
        lcd.noBacklight();
        lcd.print("Ausgeschaltet");
      }
    }
  }
  if (buttonpressedlong == false) {
    return;
  }
  buttonpressed = false; // Die Variablen wieder zurücksetzen.
  countertrybutton = 0;
  EEPROM.write(0, buzzer_active);
  EEPROM.write(1, I2C_backlight);
}
