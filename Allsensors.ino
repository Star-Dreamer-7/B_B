
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ======================================
// ROBOREGOS - PISTA A (MAZE)
// ======================================

// -------- DRIVER TB6612FNG --------

const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

const int STBY = 11;

// -------- SENSORES --------

const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;
const int IR_PIN = 7;

// -------- LEDS --------

const int pink_pin = 9;
const int orange_pin = 10;
const int yellow_pin = 3;
const int blue_pin = 12;

// -------- SENSOR RGB --------

Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// ======================================
// CONFIGURACION
// ======================================

const int VELOCIDAD = 220;
const int VELOCIDAD_GIRO = 220;

// Distancias en centimetros
const float DISTANCIA_PARED = 10.0;
const float DISTANCIA_LIBRE = 20.0;

// Tiempo de cada giro
const unsigned long TIEMPO_GIRO_PASO = 80;

// Maximo de pasos al buscar salida
const int MAX_PASOS_BUSQUEDA = 18;

// CORRECCION CONFIRMADA DE MOTORES
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = true;

// Configuracion del infrarrojo
const int IR_PARED = LOW;

// Intervalos de lectura
const unsigned long INTERVALO_RGB = 250;
const unsigned long INTERVALO_SERIAL = 400;

// ======================================
// VARIABLES
// ======================================

float distancia = -1;

float rojo = 0;
float verde = 0;
float azul = 0;

bool colorDisponible = false;

String colorDetectado = "SIN LECTURA";

unsigned long ultimaLecturaRGB = 0;
unsigned long ultimoSerial = 0;

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
    delay(200);
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
// SENSOR INFRARROJO
// ======================================

bool paredDerecha() {
  return digitalRead(IR_PIN) == IR_PARED;
}

// ======================================
// SENSOR DE COLOR RGB
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
           fabs(rojo - verde) < 0.20 &&
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
// ACTUALIZAR RGB
// ======================================

void actualizarRGB() {

  if (millis() - ultimaLecturaRGB >= INTERVALO_RGB) {

    ultimaLecturaRGB = millis();
    leerColor();
  }
}

// ======================================
// MONITOR SERIAL
// ======================================

void mostrarSensores() {

  if (millis() - ultimoSerial < INTERVALO_SERIAL) {
    return;
  }

  ultimoSerial = millis();

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
// BUSCAR CAMINO LIBRE
// ======================================

bool buscarSalida() {

  detener();

  Serial.println("BUSCANDO CAMINO LIBRE");

  for (int i = 0; i < MAX_PASOS_BUSQUEDA; i++) {

    leerUltrasonico();

    // Si encontramos suficiente espacio,
    // dejamos de girar.
    if (distancia >= DISTANCIA_LIBRE) {

      detener();

      Serial.println("CAMINO LIBRE ENCONTRADO");

      return true;
    }

    // Girar poco a poco a la izquierda
    girarIzquierda();

    delay(TIEMPO_GIRO_PASO);

    detener();

    delay(30);
  }

  detener();

  Serial.println("NO SE ENCONTRO SALIDA");

  return false;
}

// ======================================
// RESOLVER MAZE
// ======================================

void resolverMaze() {

  // Leer distancia frontal
  leerUltrasonico();

  // Actualizar RGB y monitor
  actualizarRGB();
  mostrarSensores();

  // Si no hay lectura confiable,
  // detener por seguridad.
  if (distancia < 0) {

    detener();

    Serial.println("SIN ECO - DETENIDO");

    delay(100);

    return;
  }

  // ==================================
  // PRIORIDAD 1: AVANZAR
  // ==================================

  if (distancia > DISTANCIA_PARED) {

    avanzar();

    return;
  }

  // ==================================
  // PRIORIDAD 2: EVITAR PARED
  // ==================================

  detener();

  Serial.println("PARED DETECTADA");

  bool salida = buscarSalida();

  if (salida) {

    leerUltrasonico();

    if (distancia >= DISTANCIA_LIBRE) {

      Serial.println("AVANZANDO");

      avanzar();

    } else {

      detener();
    }

  } else {

    detener();

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
