
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// Pines de los LEDs
const int purple_pin = 2;
const int orange_pin = 4;
const int yellow_pin = 8;
const int blue_pin = 12;

const int wait = 500;

// Sensor TCS34725
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// Función para apagar TODOS los LEDs
void apagarLeds() {
  digitalWrite(purple_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

void setup() {

  pinMode(purple_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  // Asegurarnos de que comiencen apagados
  apagarLeds();

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

  // Leer sensor
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {

    Serial.println("Sin lectura");

    apagarLeds();

    delay(wait);
    return;
  }

  // Convertir a proporciones
  float rojo = (float)r / c;
  float verde = (float)g / c;
  float azul = (float)b / c;

  Serial.print("R: ");
  Serial.print(rojo, 2);

  Serial.print(" | G: ");
  Serial.print(verde, 2);

  Serial.print(" | B: ");
  Serial.print(azul, 2);

  Serial.print(" | Color: ");


  apagarLeds();

  // DETECTAR COLOR

  if (rojo > 0.50 && verde < 0.22 && azul >= 0.20) {

    Serial.println("MAGENTA");

    digitalWrite(purple_pin, HIGH);
  }

  else if (rojo > 0.45 && verde > 0.25 && azul < 0.25) {

    Serial.println("NARANJA");

    digitalWrite(orange_pin, HIGH);
  }

  else if (rojo > 0.35 && verde > 0.35 && azul < 0.25) {

    Serial.println("AMARILLO");

    digitalWrite(yellow_pin, HIGH);
  }

  else if (azul > rojo && verde > rojo) {

    Serial.println("CELESTE");

    digitalWrite(blue_pin, HIGH);
  }

  else {

    Serial.println("NO IDENTIFICADO");
  }

  delay(wait);
}