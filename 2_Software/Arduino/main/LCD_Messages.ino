void LCD_Startup() { //Die LED-Startup-Anzeige
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
  lcd.print("Ueberspringen:");
  lcd.setCursor(0, 1);
  lcd.print("5 Sek. Druecken");
  delay(2500);
}