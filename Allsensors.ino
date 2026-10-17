
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ======================================
// CANDIDATES ROBOREGOS
// ULTRASONICO + RGB + LEDS
// ======================================

// ULTRASONICO
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;
const float DISTANCIA_PARED = 15.0;

// LEDS
const int pink_pin = 9;
const int orange_pin = 10;
const int yellow_pin = 3;
const int blue_pin = 12;

// SENSOR RGB
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// VARIABLES
bool colorDisponible = false;

float distancia = -1.0;
float rojo = 0.0;
float verde = 0.0;
float azul = 0.0;

String colorDetectado = "SIN LECTURA";

// ======================================
// APAGAR LEDS
// ======================================
void apagarLeds() {
  digitalWrite(pink_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

// ======================================
// PRUEBA DE LEDS
// ======================================
void probarLeds() {
  Serial.println("Probando LED ROSA");
  digitalWrite(pink_pin, HIGH);
  delay(700);
  digitalWrite(pink_pin, LOW);

  Serial.println("Probando LED NARANJA");
  digitalWrite(orange_pin, HIGH);
  delay(700);
  digitalWrite(orange_pin, LOW);

  Serial.println("Probando LED AMARILLO");
  digitalWrite(yellow_pin, HIGH);
  delay(700);
  digitalWrite(yellow_pin, LOW);

  Serial.println("Probando LED CELESTE");
  digitalWrite(blue_pin, HIGH);
  delay(700);
  digitalWrite(blue_pin, LOW);
}

// ======================================
// LEER ULTRASONICO
// ======================================
void leerUltrasonico() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  unsigned long tiempo =
    pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (tiempo == 0) {
    distancia = -1.0;
  } else {
    distancia = tiempo * 0.0343 / 2.0;
  }
}

// ======================================
// LEER SENSOR RGB
// ======================================
void leerColor() {

  if (!colorDisponible) {
    colorDetectado = "SENSOR NO DISPONIBLE";
    apagarLeds();
    return;
  }

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if (c == 0) {
    colorDetectado = "SIN LECTURA";
    apagarLeds();
    return;
  }

  rojo = (float)r / c;
  verde = (float)g / c;
  azul = (float)b / c;

  apagarLeds();

  // ROSA
  if (rojo > verde * 1.20 &&
      rojo > azul * 1.10 &&
      azul > verde * 0.85) {

    colorDetectado = "ROSA";
    digitalWrite(pink_pin, HIGH);
  }

  // AMARILLO
  else if (rojo > azul * 1.35 &&
           verde > azul * 1.35 &&
           abs(rojo - verde) < 0.20 &&
           verde > 0.20) {

    colorDetectado = "AMARILLO";
    digitalWrite(yellow_pin, HIGH);
  }

  // NARANJA
  else if (rojo > verde * 1.30 &&
           verde > azul * 1.15 &&
           verde > 0.15) {

    colorDetectado = "NARANJA";
    digitalWrite(orange_pin, HIGH);
  }

  // CELESTE
  else if (azul > rojo * 1.20 &&
           verde > rojo * 1.10) {

    colorDetectado = "CELESTE";
    digitalWrite(blue_pin, HIGH);
  }

  else {
    colorDetectado = "NO IDENTIFICADO";
  }
}

// ======================================
// MOSTRAR SENSORES
// ======================================
void mostrarSensores() {

  Serial.println("===== SENSORES =====");

  Serial.print("Distancia: ");

  if (distancia < 0) {
    Serial.println("SIN ECO");
  } else {
    Serial.print(distancia);
    Serial.println(" cm");
  }

  Serial.print("Pared enfrente: ");

  if (distancia < 0) {
    Serial.println("DESCONOCIDO");
  } else if (distancia < DISTANCIA_PARED) {
    Serial.println("SI");
  } else {
    Serial.println("NO");
  }

  Serial.print("Color detectado: ");
  Serial.println(colorDetectado);

  Serial.print("RGB: ");
  Serial.print(rojo, 3);
  Serial.print(" | ");
  Serial.print(verde, 3);
  Serial.print(" | ");
  Serial.println(azul, 3);

  Serial.println("====================");
}

// ======================================
// SETUP
// ======================================
void setup() {
  Serial.begin(9600);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(pink_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  digitalWrite(TRIGGER_PIN, LOW);
  apagarLeds();

  // Probar los cuatro LEDs
  probarLeds();

  Wire.begin();

  colorDisponible = tcs.begin();

  if (colorDisponible) {
    Serial.println("Sensor RGB listo");
  } else {
    Serial.println("ERROR: RGB no encontrado");
  }

  Serial.println("Sistema inicializado");
}

// ======================================
// LOOP PRINCIPAL
// ======================================
void loop() {
  leerUltrasonico();
  leerColor();
  mostrarSensores();

  delay(400);
}
