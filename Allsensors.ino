const int STBY = 11;

void setup() {
  Serial.begin(9600);

  pinMode(STBY, OUTPUT);
  digitalWrite(STBY, HIGH);

  Serial.println("STBY configurado en HIGH");
}

void loop() {
  Serial.print("Estado de STBY: ");
  Serial.println(digitalRead(STBY));

  delay(1000);
}
