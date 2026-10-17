
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>

// ==========================================
// ROBOREGOS - PISTA A (MAZE)
// 2 INFRARROJOS + ULTRASONICO + RETROCESO
// ==========================================

// ---------- MOTORES ----------
const int PWMA = 5;
const int AIN1 = A0;
const int AIN2 = A1;

const int PWMB = 6;
const int BIN1 = A2;
const int BIN2 = A3;

const int STBY = 11;

// ---------- ULTRASONICO ----------
const int TRIGGER_PIN = 2;
const int ECHO_PIN = 4;

// ---------- INFRARROJOS ----------
// Ambos miran hacia enfrente
const int IR_DERECHO = 7;
const int IR_IZQUIERDO = 10;

// ---------- LEDS ----------
const int pink_pin = 9;
const int orange_pin = 8;
const int yellow_pin = 3;
const int blue_pin = 12;

// ---------- SENSOR RGB ----------
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);

// ==========================================
// CONFIGURACION
// ==========================================

const int VELOCIDAD = 220;
const int VELOCIDAD_GIRO = 220;

// Retroceso suave
const int VELOCIDAD_RETROCESO = 150;

// Tiempo corto de retroceso
const unsigned long TIEMPO_RETROCESO = 220;

// Distancias del ultrasonico (cm)
const float DISTANCIA_PARED = 18.0;
const float DISTANCIA_MUY_CERCA = 12.0;
const float DISTANCIA_LIBRE = 28.0;

// Giros por pasos
const unsigned long TIEMPO_GIRO_PASO = 80;
const int MAX_PASOS_BUSQUEDA = 18;

// Ambos motores invertidos
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = true;

// Confirmado en las pruebas:
// 0 = pared, 1 = libre
const int IR_PARED = LOW;

// Intervalos
const unsigned long INTERVALO_RGB = 250;
const unsigned long INTERVALO_SERIAL = 400;

// ==========================================
// VARIABLES
// ==========================================

float distancia = -1;

float rojo = 0;
float verde = 0;
float azul = 0;

bool colorDisponible = false;
String colorDetectado = "SIN LECTURA";

unsigned long ultimaLecturaRGB = 0;
unsigned long ultimoSerial = 0;

// Evita retrocesos consecutivos
// cuando el robot no logra salir
bool retrocesoRealizado = false;

unsigned long inicioAvance = 0;

// ==========================================
// MOTORES
// ==========================================

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
  moverMotores(
    -VELOCIDAD_RETROCESO,
    -VELOCIDAD_RETROCESO
  );
}

void girarDerecha() {
  moverMotores(VELOCIDAD_GIRO, -VELOCIDAD_GIRO);
}

void girarIzquierda() {
  moverMotores(-VELOCIDAD_GIRO, VELOCIDAD_GIRO);
}

// ==========================================
// LEDS
// ==========================================

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

// ==========================================
// ULTRASONICO
// ==========================================

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

// ==========================================
// INFRARROJOS
// ==========================================

bool paredDerecha() {
  return digitalRead(IR_DERECHO) == IR_PARED;
}

bool paredIzquierda() {
  return digitalRead(IR_IZQUIERDO) == IR_PARED;
}

bool obstaculoFrontal() {
  return paredDerecha() || paredIzquierda();
}

// ==========================================
// RGB
// ==========================================

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

  if (rojo > verde * 1.20 &&
      rojo > azul * 1.10 &&
      azul > verde * 0.85) {

    colorDetectado = "ROSA";
    digitalWrite(pink_pin, HIGH);
  }

  else if (rojo > azul * 1.35 &&
           verde > azul * 1.35 &&
           fabs(rojo - verde) < 0.20 &&
           verde > 0.20) {

    colorDetectado = "AMARILLO";
    digitalWrite(yellow_pin, HIGH);
  }

  else if (rojo > verde * 1.30 &&
           verde > azul * 1.15 &&
           verde > 0.15) {

    colorDetectado = "NARANJA";
    digitalWrite(orange_pin, HIGH);
  }

  else if (azul > rojo * 1.20 &&
           verde > rojo * 1.10) {

    colorDetectado = "CELESTE";
    digitalWrite(blue_pin, HIGH);
  }

  else {
    colorDetectado = "NO IDENTIFICADO";
  }
}

void actualizarRGB() {

  if (millis() - ultimaLecturaRGB >= INTERVALO_RGB) {

    ultimaLecturaRGB = millis();
    leerColor();
  }
}

// ==========================================
// MONITOR SERIAL
// ==========================================

void mostrarSensores() {

  if (millis() - ultimoSerial < INTERVALO_SERIAL) {
    return;
  }

  ultimoSerial = millis();

  Serial.println("===== PISTA A =====");

  Serial.print("Ultrasonico: ");
  Serial.print(distancia);
  Serial.println(" cm");

  Serial.print("IR DERECHO: ");
  Serial.println(paredDerecha() ? "PARED" : "LIBRE");

  Serial.print("IR IZQUIERDO: ");
  Serial.println(paredIzquierda() ? "PARED" : "LIBRE");

  Serial.print("Color: ");
  Serial.println(colorDetectado);

  Serial.println("===================");
}

// ==========================================
// CAMINO LIBRE
// ==========================================

bool caminoLibre() {

  leerUltrasonico();

  if (distancia < DISTANCIA_LIBRE) {
    return false;
  }

  if (obstaculoFrontal()) {
    return false;
  }

  return true;
}

// ==========================================
// RETROCEDER DE PARED
// ==========================================

void retrocederDePared() {

  detener();
  delay(100);

  Serial.println("PARED CERCA: RETROCEDIENDO");

  // Solo un retroceso corto.
  // El robot NO tiene sensor trasero.
  retroceder();
  delay(TIEMPO_RETROCESO);

  detener();
  delay(150);

  retrocesoRealizado = true;

  Serial.println("RETROCESO TERMINADO");
}

// ==========================================
// BUSCAR SALIDA
// ==========================================

bool buscarSalida() {

  detener();

  Serial.println("BUSCANDO SALIDA");

  // Si detecta pared solo a la izquierda,
  // empieza girando hacia la derecha.
  // Si detecta pared a la derecha o ambos,
  // empieza girando hacia la izquierda.

  bool girarHaciaDerecha =
    paredIzquierda() && !paredDerecha();

  for (int i = 0; i < MAX_PASOS_BUSQUEDA; i++) {

    actualizarRGB();

    if (caminoLibre()) {

      detener();

      Serial.println("SALIDA ENCONTRADA");
      return true;
    }

    if (girarHaciaDerecha) {
      girarDerecha();
    } else {
      girarIzquierda();
    }

    delay(TIEMPO_GIRO_PASO);

    detener();
    delay(30);
  }

  detener();

  Serial.println("NO SE ENCONTRO SALIDA");

  return false;
}

// ==========================================
// RESOLVER MAZE
// ==========================================

void resolverMaze() {

  leerUltrasonico();

  actualizarRGB();
  mostrarSensores();

  bool irDerecho = paredDerecha();
  bool irIzquierdo = paredIzquierda();

  // Sin eco: detenerse
  if (distancia < 0) {

    detener();
    Serial.println("SIN ECO - DETENIDO");
    delay(100);
    return;
  }

  // ========================================
  // 1. CAMINO LIBRE: AVANZAR
  // ========================================

  if (distancia > DISTANCIA_PARED &&
      !irDerecho &&
      !irIzquierdo) {

    if (inicioAvance == 0) {
      inicioAvance = millis();
    }

    avanzar();

    // Solo permitir otro retroceso
    // despues de avanzar al menos 500 ms
    if (millis() - inicioAvance >= 500) {
      retrocesoRealizado = false;
    }

    return;
  }

  // ========================================
  // 2. PARED: DETENERSE
  // ========================================

  detener();
  inicioAvance = 0;

  Serial.println("OBSTACULO DETECTADO");

  // ========================================
  // 3. RETROCEDER SI ESTA MUY CERCA
  // ========================================

  bool paredMuyCerca =
    irDerecho ||
    irIzquierdo ||
    (distancia <= DISTANCIA_MUY_CERCA);

  if (paredMuyCerca && !retrocesoRealizado) {

    retrocederDePared();
  }

  // ========================================
  // 4. BUSCAR OTRA DIRECCION
  // ========================================

  bool salida = buscarSalida();

  if (salida && caminoLibre()) {

    Serial.println("AVANZANDO");
    inicioAvance = millis();
    avanzar();

  } else {

    detener();
    delay(200);
  }
}

// ==========================================
// SETUP
// ==========================================

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

  // Ultrasonico
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Infrarrojos
  pinMode(IR_DERECHO, INPUT);
  pinMode(IR_IZQUIERDO, INPUT);

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

  Serial.println("ROBOREGOS PISTA A");
  Serial.println("RETROCESO ACTIVADO");
  Serial.println("DOS INFRARROJOS ACTIVOS");

  delay(2000);
}

// ==========================================
// LOOP
// ==========================================

void loop() {
  resolverMaze();
}

