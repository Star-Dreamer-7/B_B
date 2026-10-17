
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ======================================
// ROBOREGOS CANDIDATES 2026
// PISTA A - MAZE
// DETECCION DE PAREDES Y BUSQUEDA
// ======================================

// ======================================
// MOTORES - TB6612FNG
// ======================================

const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

const int STBY = 11;

// ======================================
// SENSORES
// ======================================

const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;
const int IR_PIN = 7;

// ======================================
// LEDS
// ======================================

const int pink_pin = 9;
const int orange_pin = 10;
const int yellow_pin = 3;
const int blue_pin = 12;

// ======================================
// SENSOR DE COLOR
// ======================================

Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// ======================================
// CONFIGURACION
// ======================================

// Velocidades
const int VELOCIDAD = 130;
const int VELOCIDAD_GIRO = 160;

// Distancias del ultrasonico
const float DISTANCIA_PARED = 25.0;
const float DISTANCIA_LIBRE = 35.0;

// Sensor infrarrojo derecho
const int IR_PARED = LOW;

// Direccion de motores
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = false;

// Duracion de movimientos
const unsigned long TIEMPO_AVANCE = 30;
const unsigned long TIEMPO_GIRO_PASO = 100;

// Limite de pasos para buscar camino
const int MAX_PASOS_BUSQUEDA = 12;

// ======================================
// VARIABLES
// ======================================

bool colorDisponible = false;
bool robotBloqueado = false;

float distancia = -1;

float rojo = 0;
float verde = 0;
float azul = 0;

String colorDetectado = "SIN LECTURA";

// ======================================
// CONTROL DE MOTORES
// ======================================

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
  moverMotores(VELOCIDAD_GIRO, -VELOCIDAD_GIRO);
}

void girarIzquierda() {
  moverMotores(-VELOCIDAD_GIRO, VELOCIDAD_GIRO);
}

// ======================================
// LEDS
// ======================================

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
    delay(250);
    digitalWrite(leds[i], LOW);
  }
}

// ======================================
// SENSOR ULTRASONICO
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
    distancia = -1;
  } else {
    distancia = tiempo * 0.0343 / 2.0;
  }
}

// ======================================
// SENSOR INFRARROJO DERECHO
// ======================================

bool paredDerecha() {

  return digitalRead(IR_PIN) == IR_PARED;
}

// ======================================
// SENSOR RGB
// ======================================

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

// ======================================
// MOSTRAR SENSORES
// ======================================

void mostrarSensores() {

  Serial.println("===== PISTA A =====");

  Serial.print("Distancia frontal: ");

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

// ======================================
// AVANZAR REVISANDO DISTANCIA
// ======================================

void avanzarSeguro() {

  leerUltrasonico();

  if (distancia < 0) {

    detener();
    Serial.println("SIN ECO - DETENIDO");
    return;
  }

  if (distancia <= DISTANCIA_PARED) {

    detener();
    Serial.println("PARED CERCA - DETENIDO");
    return;
  }

  avanzar();
  delay(TIEMPO_AVANCE);
  detener();
}

// ======================================
// BUSCAR CAMINO LIBRE GIRANDO
// ======================================

bool buscarSalida() {

  detener();

  Serial.println("BUSCANDO CAMINO LIBRE");

  // Girar poco a poco hacia la izquierda
  for (int i = 0; i < MAX_PASOS_BUSQUEDA; i++) {

    leerUltrasonico();

    // Si encuentra espacio suficiente
    if (distancia >= DISTANCIA_LIBRE) {

      detener();

      Serial.println("CAMINO LIBRE ENCONTRADO");
      return true;
    }

    // Si sigue habiendo pared
    Serial.println("GIRANDO A LA IZQUIERDA");

    girarIzquierda();
    delay(TIEMPO_GIRO_PASO);

    detener();
    delay(100);
  }

  detener();

  Serial.println("NO SE ENCONTRO CAMINO LIBRE");
  return false;
}

// ======================================
// RESOLVER MAZE
// ======================================

void resolverMaze() {

  // Si ya se agoto la busqueda,
  // esperar para evitar choques repetidos.
  if (robotBloqueado) {
    detener();
    return;
  }

  leerUltrasonico();
  leerColor();
  mostrarSensores();

  // ERROR DE SENSOR
  if (distancia < 0) {

    detener();

    Serial.println("ERROR ULTRASONICO");
    delay(100);
    return;
  }

  // PARED DETECTADA
  if (distancia <= DISTANCIA_PARED) {

    detener();

    Serial.println("PARED DETECTADA");
    Serial.println("DETENER Y BUSCAR SALIDA");

    bool salidaEncontrada = buscarSalida();

    if (salidaEncontrada) {

      Serial.println("REANUDANDO AVANCE");
      avanzarSeguro();

    } else {

      robotBloqueado = true;
      Serial.println("ROBOT BLOQUEADO");
    }

    return;
  }

  // CAMINO LIBRE
  Serial.println("CAMINO LIBRE - AVANZANDO");

  avanzarSeguro();
}

// ======================================
// SETUP
// ======================================

void setup() {

  Serial.begin(9600);

  // Motores
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);

  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);

  pinMode(STBY, OUTPUT);

  // Sensores
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_PIN, INPUT);

  // LEDs
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

  digitalWrite(STBY, HIGH);

  Serial.println("ROBOREGOS MAZE INICIADO");

  delay(2000);
}

// ======================================
// LOOP PRINCIPAL
// ======================================

void loop() {

  resolverMaze();
}
