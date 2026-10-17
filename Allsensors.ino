
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ========================================
// ROBOREGOS CANDIDATES 2026
// PISTA A - INTEGRACION Y PRUEBA
// ========================================

// MOTORES - TB6612FNG
const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

const int STBY = 11;

// SENSORES
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;
const int IR_PIN = 7;

// LEDS
const int pink_pin = 9;
const int orange_pin = 10;
const int yellow_pin = 3;
const int blue_pin = 12;

// RGB
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// CONFIGURACION
const int VELOCIDAD = 110;
const float DISTANCIA_PARED = 15.0;
const int IR_PARED = LOW;

// Cambiar si alguna rueda gira al reves
const bool INVERTIR_IZQUIERDO = false;
const bool INVERTIR_DERECHO = false;

// VARIABLES
bool colorDisponible = false;
float distancia = -1;

float rojo = 0;
float verde = 0;
float azul = 0;

String colorDetectado = "SIN LECTURA";

// ========================================
// CONTROL DE MOTORES
// ========================================

void moverMotores(int izquierda, int derecha) {
  izquierda = constrain(izquierda, -255, 255);
  derecha = constrain(derecha, -255, 255);

  if (INVERTIR_IZQUIERDO) {
    izquierda = -izquierda;
  }

  if (INVERTIR_DERECHO) {
    derecha = -derecha;
  }

  digitalWrite(AIN1, izquierda > 0 ? HIGH : LOW);
  digitalWrite(AIN2, izquierda < 0 ? HIGH : LOW);

  digitalWrite(BIN1, derecha > 0 ? HIGH : LOW);
  digitalWrite(BIN2, derecha < 0 ? HIGH : LOW);

  analogWrite(PWMA, abs(izquierda));
  analogWrite(PWMB, abs(derecha));
}

void detener() {
  moverMotores(0, 0);
}

void avanzar() {
  moverMotores(VELOCIDAD, VELOCIDAD);
}

void retroceder() {
  moverMotores(-VELOCIDAD, -VELOCIDAD);
}

void girarDerecha() {
  moverMotores(VELOCIDAD, -VELOCIDAD);
}

void girarIzquierda() {
  moverMotores(-VELOCIDAD, VELOCIDAD);
}

// ========================================
// LEDS
// ========================================

void apagarLeds() {
  digitalWrite(pink_pin, LOW);
  digitalWrite(orange_pin, LOW);
  digitalWrite(yellow_pin, LOW);
  digitalWrite(blue_pin, LOW);
}

void probarLeds() {
  int leds[] = {
    pink_pin,
    orange_pin,
    yellow_pin,
    blue_pin
  };

  for (int i = 0; i < 4; i++) {
    digitalWrite(leds[i], HIGH);
    delay(400);
    digitalWrite(leds[i], LOW);
  }
}

// ========================================
// ULTRASONICO
// ========================================

void leerUltrasonico() {
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  unsigned long tiempo =
    pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (tiempo == 0) {
    distancia = -1;
  } else {
    distancia = tiempo * 0.0343 / 2.0;
  }
}

// ========================================
// INFRARROJO
// ========================================

bool paredDerecha() {
  return digitalRead(IR_PIN) == IR_PARED;
}

// ========================================
// SENSOR DE COLOR
// ========================================

void leerColor() {
  if (!colorDisponible) {
    colorDetectado = "RGB NO DISPONIBLE";
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

// ========================================
// MOSTRAR LECTURAS
// ========================================

void mostrarSensores() {
  Serial.println("===== PISTA A =====");

  Serial.print("Distancia: ");

  if (distancia < 0) {
    Serial.println("SIN ECO");
  } else {
    Serial.print(distancia);
    Serial.println(" cm");
  }

  Serial.print("Pared derecha: ");
  Serial.println(paredDerecha() ? "SI" : "NO");

  Serial.print("Color: ");
  Serial.println(colorDetectado);

  Serial.print("RGB: ");
  Serial.print(rojo, 3);
  Serial.print(" | ");
  Serial.print(verde, 3);
  Serial.print(" | ");
  Serial.println(azul, 3);

  Serial.println("===================");
}

// ========================================
// PRUEBA DE MOVIMIENTO
// ========================================

void pruebaMovimiento() {
  Serial.println("AVANZAR");
  avanzar();
  delay(300);
  detener();
  delay(800);

  Serial.println("DERECHA");
  girarDerecha();
  delay(250);
  detener();
  delay(800);

  Serial.println("IZQUIERDA");
  girarIzquierda();
  delay(250);
  detener();

  Serial.println("PRUEBA TERMINADA");
}

// ========================================
// SETUP
// ========================================

void setup() {
  Serial.begin(9600);

  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  pinMode(pink_pin, OUTPUT);
  pinMode(orange_pin, OUTPUT);
  pinMode(yellow_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);

  digitalWrite(TRIGGER_PIN, LOW);
  digitalWrite(STBY, LOW);

  detener();
  probarLeds();

  Wire.begin();
  colorDisponible = tcs.begin();

  if (colorDisponible) {
    Serial.println("RGB LISTO");
  } else {
    Serial.println("ERROR RGB");
  }

  Serial.println("INICIANDO PRUEBA");

  // HABILITAR MOTORES
  digitalWrite(STBY, HIGH);

  delay(2000);
}

// ========================================
// LOOP
// ========================================

void loop() {
  static bool pruebaTerminada = false;

  leerUltrasonico();
  leerColor();
  mostrarSensores();

  if (!pruebaTerminada) {
    pruebaTerminada = true;

    // Ejecutar solo una vez
    // con las ruedas levantadas
    pruebaMovimiento();
  }

  detener();
  delay(400);
}

