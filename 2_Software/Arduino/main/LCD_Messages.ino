void LCD_Startup() { //Die LED-Startup-Anzeige
  Serial.println("Starte das OSAMD-Projekt!");
  digitalWrite(LED_green, LOW);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Starte OSAMD");
  lcd.setCursor(4, 1);
  lcd.print("V. 1.00A");
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
  lcd.print("5 Sek. Druecken");
  delay(3500);
}

void LCD_Config() {
  int millis100pressed = 0; // Zählervariable für Knopfdruck initalisieren
  unsigned int countertrybutton = 0; //Diese Variable spreichert, wie häufig bereits geprüft wurde, ob der Knopf schon gedrückt wurde.
  bool buttonpressed = false;
  bool buttonpressedlong = false;
  Serial.println("Knopf 4 Sekunden am Stück gedrückt");
  lcd.clear();
  tone(buzzer, 440);
  lcd.setCursor(0, 0);
  lcd.print("Konfigurations-");
  lcd.setCursor(0, 1);
  lcd.print("Assistent");
  digitalWrite(LED_blue, LOW); //Die auf Blau umschalten
  delay(300);
  noTone(buzzer);
  delay(2700);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Hinweist\357ne:"); //Hinweistöne, mit Umlaut als \357 kodiert
  lcd.setCursor(0, 1);
  if (buzzer_active == true) lcd.print("Aktiviert");
  else lcd.print("Stumm");
  while ((countertrybutton <= 200) && (buttonpressed == false)) {
    if (buttonvalue() == true) {
      buttonpressed = true; //dann wird wieder aus der Funktion gesprungen
    }
    delay(100);
    countertrybutton++;
  }
  if (buttonpressed == false) {
    return;
  }
  buttonpressed = false; // Die Variablen wieder zurücksetzen.
  countertrybutton = 0;
  while (buttonpressedlong == false) {
    while ((buttonvalue() == true) && (buttonpressedlong == false)) { //Wenn der Button gedrückt ist, wird in diese Funktion gegangen
      Serial.println("Knopf gedrückt!");
      millis100pressed++;
      delay(100);
      if (millis100pressed >= 40) {
        buttonpressedlong = true;
      }
    }
    if (buttonpressedlong == false) {
      buzzer_active = !buzzer_active;
      if (buzzer_active == true) lcd.print("Aktiviert");
      else lcd.print("Stumm");
    }
  }
}
