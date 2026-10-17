#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_TCS34725.h>


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

// SENSOR RGB
Adafruit_TCS34725 tcs(
  TCS34725_INTEGRATIONTIME_60MS,
  TCS34725_GAIN_1X
);


// CONFIGURACION

// Velocidad de avance
const int VELOCIDAD = 130;

// Velocidad de giro
const int VELOCIDAD_GIRO = 160;

// Distancia minima antes de una pared
const float DISTANCIA_PARED = 15.0;

// IR: cambiar a HIGH si tu sensor detecta
// paredes con valor 1
const int IR_PARED = LOW;

// Corregir movimiento hacia atras
const bool INVERTIR_IZQUIERDO = true;
const bool INVERTIR_DERECHO = false;

// AJUSTAR SEGUN TU ROBOT
// Tiempos aproximados, NO calibrados
const unsigned long TIEMPO_GIRO_90 = 450;
const unsigned long TIEMPO_AVANCE_CORTO = 250;

// VARIABLES
bool colorDisponible = false;

float distancia = -1;
float rojo = 0;
float verde = 0;
float azul = 0;

String colorDetectado = "SIN LECTURA";


// CONTROL DE MOTORES

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

// LEDS

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


// ULTRASONICO

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


// INFRARROJO DERECHO

bool paredDerecha() {
  return digitalRead(IR_PIN) == IR_PARED;
}

// SENSOR RGB

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
  // ROJO - CHECKPOINT FINAL
if (rojo > verde * 1.45 &&
    rojo > azul * 1.45) {

  colorDetectado = "ROJO";
}

// ROSA
else if (rojo > verde * 1.20 &&
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

// MOSTRAR SENSORES

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

// MOVIMIENTOS CALIBRABLES

void giro90Derecha() {

  detener();
  delay(150);

  girarDerecha();
  delay(TIEMPO_GIRO_90);

  detener();
  delay(150);
}

void giro90Izquierda() {

  detener();
  delay(150);

  girarIzquierda();
  delay(TIEMPO_GIRO_90);

  detener();
  delay(150);
}

void giro180() {

  detener();
  delay(150);

  girarDerecha();
  delay(TIEMPO_GIRO_90 * 2);

  detener();
  delay(150);
}


// WALL FOLLOWER - MANO DERECHA

void resolverMaze() {

  leerUltrasonico();
  leerColor();
  mostrarSensores();

  // Si no hay lectura frontal confiable,
  // detener para evitar choques.
  if (distancia < 0) {
    detener();
    Serial.println("SIN ECO - DETENIDO");
    delay(150);
    return;
  }

  bool frenteLibre = distancia > DISTANCIA_PARED;
  bool derechaLibre = !paredDerecha();

  // PRIORIDAD 1: CAMINO LIBRE A LA DERECHA
  if (derechaLibre) {

    Serial.println("CAMINO A LA DERECHA");

    detener();
    delay(100);

    giro90Derecha();

    // Comprobar espacio tras girar
    leerUltrasonico();

    if (distancia > DISTANCIA_PARED) {
      avanzar();
      delay(TIEMPO_AVANCE_CORTO);
    } else {
      detener();
    }
  }

  // PRIORIDAD 2: AVANZAR DE FRENTE
  else if (frenteLibre) {

    Serial.println("AVANZANDO");

    avanzar();
    delay(100);
  }

  // PRIORIDAD 3: GIRAR A LA IZQUIERDA
  else {

    Serial.println("PARED ENFRENTE Y DERECHA");

    giro90Izquierda();

    leerUltrasonico();

    // Si tambien hay pared a la izquierda,
    // girar otros 90 grados (media vuelta total)
    if (distancia > 0 &&
        distancia <= DISTANCIA_PARED) {

      Serial.println("CALLEJON SIN SALIDA");
      giro90Izquierda();
    }
  }
}


// SETUP

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

  digitalWrite(STBY, HIGH);

  Serial.println("WALL FOLLOWER INICIADO");

  delay(2000);
}
// LOOP PRINCIPAL
void loop() {
  resolverMaze();
}



