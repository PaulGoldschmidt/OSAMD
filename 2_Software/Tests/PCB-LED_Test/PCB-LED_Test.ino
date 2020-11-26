/*
  OSAMD-LED-Test
  Hier sollte die LED komplett weiß leuchten!
*/

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {  // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(11, LOW);    // turn the LED off by making the voltage LOW
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  delay(100);                       // wait for a second
}
