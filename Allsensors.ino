
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ======================================
// ROBOREGOS CANDIDATES 2026
// PISTA A - MAZE
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
// SENSOR RGB
// ======================================

Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// ======================================
// CONFIGURACION
// ======================================

// Velocidades
const int VELOCIDAD = 190;
const int VELOCIDAD_GIRO = 220;

// Distancias del ultrasonico
const float DISTANCIA_PARED = 10.0;
const float DISTANCIA_LIBRE = 20.0;

// Infrarrojo derecho
const int IR_PARED = LOW;

// Direccion de motores
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = false;

// Tiempos
const unsigned long TIEMPO_AVANCE = 30;
const unsigned long TIEMPO_GIRO_PASO = 80;

// Limite de busqueda
const int MAX_PASOS_BUSQUEDA = 18;

// Lectura RGB cada 250 ms
const unsigned long INTERVALO_RGB = 250;

// ======================================
// VARIABLES
// ======================================

bool colorDisponible = false;

float distancia = -1;
float rojo = 0;
float verde = 0;
float azul = 0;

String colorDetectado = "SIN LECTURA";

unsigned long ultimaLecturaRGB = 0;

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
// ULTRASONICO
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
// INFRARROJO DERECHO
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

  Serial.println("===================");
}

// ======================================
// ACTUALIZAR RGB SIN RETRASAR TANTO
// ======================================

void actualizarRGB() {

  if (millis() - ultimaLecturaRGB >= INTERVALO_RGB) {

    ultimaLecturaRGB = millis();
    leerColor();
  }
}

// ======================================
// AVANZAR SI EL FRENTE ESTA LIBRE
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

  // Mantener motores avanzando
  avanzar();
}

// ======================================
// BUSCAR CAMINO LIBRE GIRANDO
// ======================================

bool buscarSalida() {

  detener();
  Serial.println("BUSCANDO CAMINO");

  for (int i = 0; i < MAX_PASOS_BUSQUEDA; i++) {

    leerUltrasonico();

    // Solo detener el giro si hay
    // suficiente espacio al frente
    if (distancia >= DISTANCIA_LIBRE) {

      detener();
      Serial.println("CAMINO LIBRE ENCONTRADO");
      return true;
    }

    // Girar un poco hacia la izquierda
    girarIzquierda();
    delay(TIEMPO_GIRO_PASO);
    detener();
  }

  detener();
  Serial.println("NO SE ENCONTRO CAMINO");
  return false;
}

// ======================================
// RESOLVER MAZE
// ======================================

void resolverMaze() {

  leerUltrasonico();
  actualizarRGB();

  // Sin lectura
  if (distancia < 0) {

    detener();
    Serial.println("ERROR ULTRASONICO");
    return;
  }

  // PRIORIDAD: AVANZAR SI EL FRENTE ESTA LIBRE
  if (distancia > DISTANCIA_PARED) {

    avanzar();
    return;
  }

  // SOLO GIRAR SI HAY PARED ENFRENTE
  detener();

  Serial.println("PARED FRONTAL - BUSCANDO SALIDA");

  bool salida = buscarSalida();

  if (salida) {

    // Confirmar que sigue libre
    leerUltrasonico();

    if (distancia >= DISTANCIA_LIBRE) {

      Serial.println("AVANZANDO");
      avanzar();
    }

  } else {

    detener();
    Serial.println("SIN SALIDA CONFIRMADA");
    delay(200);
  }
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

