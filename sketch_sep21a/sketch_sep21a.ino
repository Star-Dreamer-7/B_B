#include <Wire.h>
#include <Adafruit_TCS34725.h>

// Configuracion del sensor
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

void setup() {

  Serial.begin(9600);

  Serial.println("Inicializando...");

  if (!tcs.begin()) {
    Serial.println("No se encontro el sensor");
    while (1);
  }

  Serial.println("Sensor listo");
}

void loop() {

  uint16_t r, g, b, c;

  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    Serial.println("Sin lectura");
    delay(500);
    return;
  }

  // Convertir los valores a proporciones
  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;

  // Mostrar valores
  Serial.print("R: ");
  Serial.print(rojo, 2);

  Serial.print(" | G: ");
  Serial.print(verde, 2);

  Serial.print(" | B: ");
  Serial.print(azul, 2);

  Serial.print(" | Color: ");

  // MAGENTA
  if (rojo > 0.50 && verde < 0.22 && azul >= 0.20) {
    Serial.println("MAGENTA");
  }

  // ROJO
  else if (rojo > 0.45 && verde < 0.30 && azul < 0.20) {
    Serial.println("ROJO");
  }

  // NARANJA
  else if (rojo > 0.45 && verde > 0.25 && azul < 0.25) {
    Serial.println("NARANJA");
  }

  // VERDE
  else if (verde > rojo && verde > azul) {
    Serial.println("VERDE");
  }

  // AMARILLO
  else if (rojo > 0.35 && verde > 0.35 && azul < 0.25) {
    Serial.println("AMARILLO");
  }

  // TURQUESA
  else if (azul > rojo && verde > rojo) {
    Serial.println("TURQUESA");
  }

  else {
    Serial.println("NO IDENTIFICADO");
  }

  delay(500);
}