void loop() {

  detener();
  delay(2000);

  Serial.println("PRUEBA: AVANZAR");

  avanzar();
  delay(700);

  detener();

  // Termina la prueba hasta reiniciar
  while (true) {
    detener();
  }
}
